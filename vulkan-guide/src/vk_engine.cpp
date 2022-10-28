#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include "VkBootstrap.h"

#include <iostream>
#include <fstream>
#include <functional>

#include "vk_pipeline.h"

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)

#pragma region init

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it.
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags);

	// load the core vulkan structures
	init_vulkan();

	// create the swapchain
	init_swapchain();

	init_commands();

	init_default_renderpass();

	init_framebuffers();

	init_sync_structures();

	init_pipelines();

	// everything went fine
	_isInitialized = true;
}

void VulkanEngine::init_vulkan()
{
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	vkb::Instance vkb_inst = builder.set_app_name("Example Vulkan Application")
								 .request_validation_layers(true)
								 .require_api_version(1, 1, 0)
								 .use_default_debug_messenger()
								 .build()
								 .value();

	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;

	// get the surface of the window we opened with SDL
	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

	// Select a gpu that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{vkb_inst};
	vkb::PhysicalDevice physicalDevice = selector
											 .set_minimum_version(1, 1)
											 .set_surface(_surface)
											 .select()
											 .value();

	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	vkb::Device vkbDevice = deviceBuilder.build().value();

	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::init_swapchain()
{
	vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

	vkb::Swapchain vkbSwapchain = swapchainBuilder
									  .use_default_format_selection()
									  // use vsync present mode
									  .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
									  .set_desired_extent(_windowExtent.width, _windowExtent.height)
									  .build()
									  .value();

	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();
	_swapchainImageFormat = vkbSwapchain.image_format;

	_mainDeletionQueue.push_function([=](){
		vkDestroySwapchainKHR(_device,_swapchain,nullptr);
	});
}

void VulkanEngine::init_commands()
{
	// create command pool for commands submitted to the graphics queue
	// the command pool will be one that can submit graphics command
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_finfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	// allocate the default command buffer that we will use for rendering
	VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));

	_mainDeletionQueue.push_function([=](){
		vkDestroyCommandPool(_device,_commandPool,nullptr);
	});
}

void VulkanEngine::init_default_renderpass()
{
	// renderpass will use this color attachment
	VkAttachmentDescription color_attachment{};
	// the attachment will have the format required by the swapchain
	color_attachment.format = _swapchainImageFormat;
	// only one sample, no antialiasing
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// we dont care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// we dont know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	// attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// we are going to create 1 subpass, because a renderpass can only exists if it has at least one subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	// connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;

	// connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

	_mainDeletionQueue.push_function([=](){
		vkDestroyRenderPass(_device,_renderPass,nullptr);
	});
}

void VulkanEngine::init_framebuffers()
{
	// create the framebuffers for the swapchain images.
	// This will connect the render-pass to the images for rendering.
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass,_windowExtent);

	// grab how many images we have in the swapchains
	const uint32_t swapchain_imagecount = _swapchainImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	// create framebuffers for each of the swapchain image views
	for (int i = 0; i < _swapchainImageViews.size(); i++)
	{
		fb_info.pAttachments = &_swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));

		_mainDeletionQueue.push_function([=](){
			vkDestroyFramebuffer(_device,_framebuffers[i],nullptr);
			vkDestroyImageView(_device,_swapchainImageViews[i],nullptr);
		});
	}
}

void VulkanEngine::init_sync_structures()
{
	// create synchronisation structures

	// we want to create the fence with the create signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

	_mainDeletionQueue.push_function([=](){
		vkDestroyFence(_device,_renderFence,nullptr);
	});

	// for the semaphore, no flag needed
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));

	_mainDeletionQueue.push_function([=](){
		vkDestroySemaphore(_device, _presentSemaphore,nullptr);
		vkDestroySemaphore(_device,_renderSemaphore,nullptr);
	});
}

bool VulkanEngine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{
	//open the file, with cursor at the end
	std::ifstream file(filePath,std::ios::ate | std::ios::binary);
		
	if (!file.is_open())
	{
		return false;
	}

	//find the size of the file by looking up the location of the cursor
	//because it is at the end, it gives the size direcly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spir-v expects the buffer to be on uint32, so we have to reserve an int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at the beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(),fileSize);

	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multiply the ints in the buffer by sizeof uint32 to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device,& createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return false;
	}
	*outShaderModule = shaderModule;
	
	return true;
}


void VulkanEngine::init_pipelines()
{
	VkShaderModule triangleFragShader;
	if (!load_shader_module("../shaders/colored_triangle.frag.spv", &triangleFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module." <<std::endl;
	}
	else
	{
		std::cout << "Triangle fragment shader successfully loaded." << std::endl;
	}

	VkShaderModule triangleVertexShader;
	if (!load_shader_module("../shaders/colored_triangle.vert.spv", &triangleVertexShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Triangle vertex shader successfully loaded." << std::endl;
	}
	VkShaderModule redTriangleFragShader;
	if (!load_shader_module("../shaders/triangle.frag.spv", &redTriangleFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module." <<std::endl;
	}
	else
	{
		std::cout << "Red Triangle fragment shader successfully loaded." << std::endl;
	}

	VkShaderModule redTriangleVertexShader;
	if (!load_shader_module("../shaders/triangle.vert.spv", &redTriangleVertexShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Red Triangle vertex shader successfully loaded." << std::endl;
	}
	
	//build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything than empty default
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::pipeline_layout_create_info();

	VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_trianglePipelineLayout));

	//build the stage create info for both vertex and fragment stages.
	PipelineBuilder pipelineBuilder;

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,triangleVertexShader)
	);

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,triangleFragShader)
	);

	//vertex input controls how to read vertices from vertex buffers, not used yet
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from swapchain extents
	pipelineBuilder._viewport.x = 0.f;
	pipelineBuilder._viewport.y = 0.f;
	pipelineBuilder._viewport.width = (float) _windowExtent.width;
	pipelineBuilder._viewport.height = (float) _windowExtent.height;
	pipelineBuilder._viewport.minDepth = 0.f;
	pipelineBuilder._viewport.maxDepth = 1.f;

	pipelineBuilder._scissor.offset = { 0, 0};
	pipelineBuilder._scissor.extent = _windowExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

	//multisampling not used
	pipelineBuilder._multisampling = vkinit:: multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA 
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

	//use the triangle layout
	pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

	//build the pipeline
	_trianglePipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	//clear the shader stages for the builder
	pipelineBuilder._shaderStages.clear();

	//add the other shaders
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,redTriangleVertexShader)
	);
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,redTriangleFragShader)
	);

	_redTrianglePipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	vkDestroyShaderModule(_device,redTriangleVertexShader,nullptr);
	vkDestroyShaderModule(_device, redTriangleFragShader,nullptr);
	vkDestroyShaderModule(_device, triangleVertexShader,nullptr);
	vkDestroyShaderModule(_device, triangleFragShader,nullptr);

	_mainDeletionQueue.push_function([=](){
		//destroy the 2 pipelines we have created
		vkDestroyPipeline(_device,_redTrianglePipeline,nullptr);
		vkDestroyPipeline(_device,_trianglePipeline,nullptr);

		//destroy the pipeline layout that they use
		vkDestroyPipelineLayout(_device, _trianglePipelineLayout,nullptr);
	});

}

#pragma endregion init

void VulkanEngine::cleanup()
{
	if (_isInitialized)
	{

		//make sur the gpu has stopped doing its things

		vkWaitForFences(_device,1,&_renderFence,true,1000000000);

		_mainDeletionQueue.flush();
		
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);

		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::draw()
{
	// wait until the gpu has finished rendering the last frame, Timeout of 1 seconds.
	VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &_renderFence));

	// request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore,nullptr,&swapchainImageIndex));

	VkCommandBuffer cmd = _mainCommandBuffer;

	// begin the command buffer recording.
	// we will use this command buffer exactly once, so we want to let vulkan know it
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// make a clear color from frame number
	// this will flash with a 120*pi frame period.
	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

	// start the main renderpass
	// we will use the clear color from above and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass,_windowExtent,_framebuffers[swapchainImageIndex]);

	// connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	if (_selectedShader == 0)
	{
		vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,_trianglePipeline);
	}
	else
	{
		vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,_redTrianglePipeline);		
	}
	

	vkCmdDraw(cmd,3,1,0,0);

	// finalize the render pass
	vkCmdEndRenderPass(cmd);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	// prepare the submission to the queue.
	// we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::submit_info(&cmd);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &_presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &_renderSemaphore;

	// submit command buffer to the queue and execute it.
	// renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

	// this will put the image we just rendered into the visible window
	// we want to wait on the _renderSemaphore for that
	// as it is necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::present_info();

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_renderSemaphore;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_graphicsQueue,&presentInfo));

	//increases the number of frames drawn
	_frameNumber++;
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	// main loop
	while (!bQuit)
	{
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			// close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_QUIT)
				bQuit = true;
			else if (e.key.keysym.sym == SDLK_SPACE)
			{
				_selectedShader +=1;
				if (_selectedShader > 1)
				{
					_selectedShader = 0;
				}				
			}			
		}
		draw();
	}
}

