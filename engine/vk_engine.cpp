#include "vk_engine.h"
#include "vk_instance_builder.h"
#include "vk_device_selector.h"
#include "vk_device_builder.h"
#include "vk_swapchain.h"
#include <SDL.h>
#include <SDL_vulkan.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include <vk_initializers.h>

#include <iostream>
#include <fstream>
#include <functional>

#include <glm/gtx/transform.hpp>

#include "vk_pipeline.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#ifndef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

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

	init_descriptors();

	init_pipelines();

	init_imgui();

	load_images();

	load_meshes();

	init_scene();

	// everything went fine
	_isInitialized = true;
}



void VulkanEngine::init_vulkan()
{
	unsigned int nbExtensions;
	SDL_Vulkan_GetInstanceExtensions(_window,&nbExtensions,nullptr);
	std::vector<const char*> extensions(nbExtensions);
	SDL_Vulkan_GetInstanceExtensions(_window,&nbExtensions,extensions.data());
	VulkanInstanceBuilder builder;
	
	// make the vulkan instance, with basic debug features
	VulkanInstance vbInst = builder.setAppName("VulkanEngine Demo")
													   .enableValidationLayers(enableValidationLayers)
													   .setApiVersion(1, 1, 0)
													   .addExtensions(extensions)
													   .setEngineName("ForestierEngiiiine")
													   .build()
													   .value();

	_instance = vbInst._instance;
	_debug_messenger = vbInst._debugMessenger;

	// get the surface of the window we opened with SDL
	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

	// Select a gpu that can write to the SDL surface and supports Vulkan 1.1
	VulkanDeviceSelector selector(vbInst,_surface);

	VulkanPhysicalDevice physicalDevice = selector
											.setApiVersion(1, 1, 0)
											.select()
											.value();

	VulkanDeviceBuilder deviceBuilder(physicalDevice);
	VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures{};
	shaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	shaderDrawParametersFeatures.pNext = nullptr;
	shaderDrawParametersFeatures.shaderDrawParameters = VK_TRUE;
	VulkanDevice dev = deviceBuilder.addPNext(&shaderDrawParametersFeatures).build().value();

	_device = dev._device;
	_chosenGPU = physicalDevice._device;

	_graphicsQueueFamily = physicalDevice._graphicsQueueFamily;
	_presentQueueFamily = physicalDevice._presentQueueFamily;
    vkGetDeviceQueue(_device, _graphicsQueueFamily, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, _presentQueueFamily, 0, &_presentQueue);

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo,&_allocator);

	_mainDeletionQueue.push_function([&](){
		vmaDestroyAllocator(_allocator);
	});

	_gpuProperties = physicalDevice._properties;

	std::cout << " The GPU has a minimum buffer alignment of " << _gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
}

void VulkanEngine::init_imgui()
{
	//1: create descriptor pool for imgui
	//the size of the pool is very oversize, but it's copied from imgui demo itself
	VkDescriptorPoolSize poolSizes[]={
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(_device,&poolInfo,nullptr,&imguiPool));

	//2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for sdl
	ImGui_ImplSDL2_InitForVulkan(_window);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = _instance;
	initInfo.PhysicalDevice = _chosenGPU;
	initInfo.Device = _device;
	initInfo.Queue = _graphicsQueue;
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = 3;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&initInfo,_renderPass);
	
	//execute a gpu command to upload imgui font textures
	immediate_submit([&](VkCommandBuffer cmd){
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	});

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	//add the destruction of the imgui created structures
	_mainDeletionQueue.push_function([=](){
		vkDestroyDescriptorPool(_device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	});
}

void VulkanEngine::init_swapchain()
{
	VulkanSwapchainBuilder swapchainBuilder(_chosenGPU,_device,_surface, _graphicsQueueFamily,_graphicsQueueFamily);

	VulkanSwapchain vkbSwapchain = swapchainBuilder
									  // use vsync present mode
									  .setPresentMode(VK_PRESENT_MODE_FIFO_KHR)
									  .setExtent(_windowExtent.width, _windowExtent.height)
									  .build()
									  .value();

	_swapchain = vkbSwapchain._swapchain;
	_swapchainImages = vkbSwapchain._images;
	_swapchainImageViews = vkbSwapchain._imagesviews;
	_swapchainImageFormat = vkbSwapchain._imageFormat;

	_mainDeletionQueue.push_function([=](){
		vkDestroySwapchainKHR(_device,_swapchain,nullptr);
	});

	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		_windowExtent.width,
		_windowExtent.height,
		1
	};

	//hardcoding the depth format to 32 bit float
	_depthFormat = VK_FORMAT_D32_SFLOAT;

	//the depth image will be an image with the format we selected and depth attachment usage flag
	VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthFormat,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,depthImageExtent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo{};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(_allocator,&dimg_info,&dimg_allocinfo,&_depthImage._image,&_depthImage._allocation,nullptr);

	//build an image view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthFormat,_depthImage._image,VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(_device,&dview_info,nullptr,&_depthImageView));

	//add to deletion queues
	_mainDeletionQueue.push_function([=](){
		vkDestroyImageView(_device,_depthImageView,nullptr);
		vmaDestroyImage(_allocator,_depthImage._image,_depthImage._allocation);
	});
}

void VulkanEngine::init_commands()
{
	// create command pool for commands submitted to the graphics queue
	// the command pool will be one that can submit graphics command
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_finfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		// allocate the default command buffer that we will use for rendering
		VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));

		_mainDeletionQueue.push_function([=](){
			vkDestroyCommandPool(_device,_frames[i]._commandPool,nullptr);
		});
	}

	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_finfo(_graphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(_device,&uploadCommandPoolInfo,nullptr,&_uploadContext._commandPool));

	_mainDeletionQueue.push_function([=](){
		vkDestroyCommandPool(_device,_uploadContext._commandPool,nullptr);
	});

	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool,1);
	VK_CHECK(vkAllocateCommandBuffers(_device,&cmdAllocInfo,&_uploadContext._commandBuffer));
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

	VkAttachmentDescription depth_attachment{};
	depth_attachment.flags = 0;
	depth_attachment.format = _depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref{};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// we are going to create 1 subpass, because a renderpass can only exists if it has at least one subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkAttachmentDescription attachments[2] = {color_attachment,depth_attachment};

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency{};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	// connect the attachments to the info
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = attachments;

	// connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkSubpassDependency dependencies[2] = { dependency,depth_dependency};

	render_pass_info.dependencyCount = 2;
	render_pass_info.pDependencies = dependencies;

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
		VkImageView attachments[2] = {_swapchainImageViews[i], _depthImageView};;
		fb_info.attachmentCount = 2;
		fb_info.pAttachments = attachments;
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

	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();
	VK_CHECK(vkCreateFence(_device,&uploadFenceCreateInfo,nullptr,&_uploadContext._uploadFence));

	_mainDeletionQueue.push_function([=](){
		vkDestroyFence(_device,_uploadContext._uploadFence,nullptr);
	});

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();
	
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

		_mainDeletionQueue.push_function([=](){
			vkDestroyFence(_device,_frames[i]._renderFence,nullptr);
		});

		// for the semaphore, no flag needed

		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

		_mainDeletionQueue.push_function([=](){
			vkDestroySemaphore(_device, _frames[i]._presentSemaphore,nullptr);
			vkDestroySemaphore(_device, _frames[i]._renderSemaphore,nullptr);
		});
	}
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

void VulkanEngine::init_descriptors()
{
	const size_t _sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

	_sceneParametersBuffer = create_buffer(_sceneParamBufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);

	//create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,10},		
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,10},
		//add combined image sampler descriptor types to the pool
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,10}		
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.maxSets = 10;
	poolInfo.poolSizeCount = (uint32_t) sizes.size();
	poolInfo.pPoolSizes = sizes.data();

	VK_CHECK(vkCreateDescriptorPool(_device,&poolInfo,nullptr,&_descriptorPool));

	//information about the bindings
	VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,1);
	VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);

	VkDescriptorSetLayoutBinding bindings[] = {cameraBind,sceneBind};

	VkDescriptorSetLayoutCreateInfo setinfo{};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	
	setinfo.bindingCount = 2;
	setinfo.flags = 0;
	setinfo.pBindings = bindings;

	VK_CHECK(vkCreateDescriptorSetLayout(_device,&setinfo,nullptr,&_globalSetLayout));
	
	VkDescriptorSetLayoutCreateInfo set2info{};
	set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set2info.pNext = nullptr;
	set2info.bindingCount = 1;
	set2info.flags = 0;
	set2info.pBindings = &objectBind;

	VK_CHECK(vkCreateDescriptorSetLayout(_device,&set2info,nullptr,&_objectSetLayout));

	VkDescriptorSetLayoutBinding textureBind = vkinit:: descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);

	VkDescriptorSetLayoutCreateInfo set3info{};
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.pNext = nullptr;
	set3info.bindingCount = 1;
	set3info.pBindings = &textureBind;
	set3info.flags = 0;

	vkCreateDescriptorSetLayout(_device,&set3info,nullptr,&_singleTextureSetLayout);


	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		const int MAX_OBJECTS = 10000;
		_frames[i]._objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);

		_frames[i]._cameraBuffer = create_buffer(sizeof(GPUCameraData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
	
		//allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &_globalSetLayout;

		vkAllocateDescriptorSets(_device,&allocInfo,&_frames[i]._globalDescriptor);

		//allocate the descriptor set that will point to object buffer
		VkDescriptorSetAllocateInfo objectSetAlloc{};
		objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		objectSetAlloc.pNext = nullptr;
		objectSetAlloc.descriptorPool = _descriptorPool;
		objectSetAlloc.descriptorSetCount = 1;
		objectSetAlloc.pSetLayouts = &_objectSetLayout;

		vkAllocateDescriptorSets(_device,&objectSetAlloc,&_frames[i].__objectDescriptor);

		//information about the buffer we want to point at in the descriptor
		VkDescriptorBufferInfo cameraInfo{};
		//bind camera buffer
		cameraInfo.buffer = _frames[i]._cameraBuffer._buffer;
		//at 0 offset
		cameraInfo.offset = 0;
		//of the size of a camera data struct
		cameraInfo.range = sizeof(GPUCameraData);
		
		VkDescriptorBufferInfo sceneInfo{};
		sceneInfo.buffer = _sceneParametersBuffer._buffer;
		//for non dynamic buffer 
		//sceneInfo.offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * i;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo{};
		objectBufferInfo.buffer = _frames[i]._objectBuffer._buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

		VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,_frames[i]._globalDescriptor,&cameraInfo,0);

		VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,_frames[i]._globalDescriptor,&sceneInfo,1);
		
		VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,_frames[i].__objectDescriptor,&objectBufferInfo,0);
		
		VkWriteDescriptorSet setWrites[] = {cameraWrite,sceneWrite, objectWrite};

		vkUpdateDescriptorSets(_device,3,setWrites,0,nullptr);
	}


	_mainDeletionQueue.push_function([&](){
		vmaDestroyBuffer(_allocator,_sceneParametersBuffer._buffer,_sceneParametersBuffer._allocation);
		vkDestroyDescriptorSetLayout(_device,_objectSetLayout,nullptr);
		vkDestroyDescriptorSetLayout(_device,_globalSetLayout,nullptr);
		vkDestroyDescriptorSetLayout(_device,_singleTextureSetLayout,nullptr);
		vkDestroyDescriptorPool(_device,_descriptorPool,nullptr);
		for (int i = 0; i < FRAME_OVERLAP; i++)
		{				
			vmaDestroyBuffer(_allocator,_frames[i]._cameraBuffer._buffer,_frames[i]._cameraBuffer._allocation);			
			vmaDestroyBuffer(_allocator,_frames[i]._objectBuffer._buffer,_frames[i]._objectBuffer._allocation);			
		}	
	});
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

	VkPipelineLayout trianglePipelineLayout;

	VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &trianglePipelineLayout));

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
	pipelineBuilder._pipelineLayout = trianglePipelineLayout;

	//add depth testing
	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true,true,VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the pipeline
	VkPipeline trianglePipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	create_material(trianglePipeline,trianglePipelineLayout,"triangle");

	//clear the shader stages for the builder
	pipelineBuilder._shaderStages.clear();

	//add the other shaders
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,redTriangleVertexShader)
	);
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,redTriangleFragShader)
	);

	VkPipeline redTrianglePipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	create_material(redTrianglePipeline,trianglePipelineLayout,"red triangle");
	//create the mesh pipeline layout
	VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkinit::pipeline_layout_create_info();

	//setup push constants
	VkPushConstantRange pushConstant;
	//this push constant range starts at the beginning
	pushConstant.offset = 0;
	//this push constant range takes up the size of a meshpushconstants struct
	pushConstant.size = sizeof(MeshPushConstants);
	//this push constant range is accessible only in the vertex shader
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	meshPipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	meshPipelineLayoutInfo.pushConstantRangeCount = 1;

	VkDescriptorSetLayout setLayouts[] = {_globalSetLayout, _objectSetLayout};

	meshPipelineLayoutInfo.setLayoutCount = 2;
	meshPipelineLayoutInfo.pSetLayouts = setLayouts;

	VkPipelineLayout meshPipelineLayout;

	VK_CHECK(vkCreatePipelineLayout(_device,&meshPipelineLayoutInfo,nullptr,&meshPipelineLayout));

	//build the mesh pipeline
	
	VertexInputDescription vertexDescription = Vertex::get_vertex_description();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
	
	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	//clear the shader stages for the builder
	pipelineBuilder._shaderStages.clear();

	//compile mesh vertex shader

	VkShaderModule meshVertShader;
	if (!load_shader_module("../shaders/tri_mesh.vert.spv",&meshVertShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Mesh Triangle vertex shader successfully loaded" << std::endl;
	}

	VkShaderModule meshFragShader;
	if (!load_shader_module("../shaders/default_lit.frag.spv",&meshFragShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Mesh Triangle fragment shader successfully loaded" << std::endl;
	}

	//add the other shaders
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,meshVertShader)
	);

	//make sure that triangleFragShader is holding the compiled colored_triangle.frag
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,meshFragShader)
	);

	pipelineBuilder._pipelineLayout = meshPipelineLayout;

	//build the mesh triangle pipeline
	VkPipeline meshPipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	//create a default material with the mesh pipeline
	create_material(meshPipeline,meshPipelineLayout, "defaultmesh");

	//textured pipeline
	VkShaderModule texturedFragShader;
	if (!load_shader_module("../shaders/textured_lit.frag.spv",&texturedFragShader))
	{
		std::cout << "Error when building the textured mesh shader" << std::endl;
	}

	//create pipeline layout for the textured mesh which has 3 descriptor sets
	VkPipelineLayoutCreateInfo texturedPipelineLayoutInfo = meshPipelineLayoutInfo;

	VkDescriptorSetLayout texturedSetLayouts[] = {_globalSetLayout, _objectSetLayout, _singleTextureSetLayout};

	texturedPipelineLayoutInfo.setLayoutCount = 3;
	texturedPipelineLayoutInfo.pSetLayouts = texturedSetLayouts;

	VkPipelineLayout texturedPipeLayout;
	VK_CHECK(vkCreatePipelineLayout(_device,&texturedPipelineLayoutInfo,nullptr,&texturedPipeLayout));

	pipelineBuilder._pipelineLayout = texturedPipeLayout;

	pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,meshVertShader)
	);

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,texturedFragShader)
	);

	VkPipeline texPipeline = pipelineBuilder.build_pipeline(_device,_renderPass);

	create_material(texPipeline,texturedPipeLayout,"texturedmesh");


	vkDestroyShaderModule(_device,texturedFragShader,nullptr);
	vkDestroyShaderModule(_device,meshFragShader,nullptr);
	vkDestroyShaderModule(_device,meshVertShader,nullptr);
	vkDestroyShaderModule(_device,redTriangleVertexShader,nullptr);
	vkDestroyShaderModule(_device, redTriangleFragShader,nullptr);
	vkDestroyShaderModule(_device, triangleVertexShader,nullptr);
	vkDestroyShaderModule(_device, triangleFragShader,nullptr);

	_mainDeletionQueue.push_function([=](){
		//destroy the 2 pipelines we have created
		vkDestroyPipeline(_device,redTrianglePipeline,nullptr);
		vkDestroyPipeline(_device,trianglePipeline,nullptr);
		vkDestroyPipeline(_device,meshPipeline,nullptr);
		vkDestroyPipeline(_device,texPipeline,nullptr);

		//destroy the pipeline layout that they use
		vkDestroyPipelineLayout(_device, trianglePipelineLayout,nullptr);
		vkDestroyPipelineLayout(_device, meshPipelineLayout,nullptr);
		vkDestroyPipelineLayout(_device, texturedPipeLayout,nullptr);
	});

}

void VulkanEngine::load_meshes()
{
	Mesh triangleMesh;
	//make the array 3 vertices long
	triangleMesh._vertices.resize(3);

	//vertex positions
	triangleMesh._vertices[0].position = {1.f,1.f,0.f};
	triangleMesh._vertices[1].position = {-1.f,1.f,0.f};
	triangleMesh._vertices[2].position = {0.f,-1.f,0.f};

	//vertex colors all green
	triangleMesh._vertices[0].color = {0.f,1.f,0.f};
	triangleMesh._vertices[1].color = {0.f,1.f,0.f};
	triangleMesh._vertices[2].color = {0.f,1.f,0.f};

	Mesh monkeyMesh;
	monkeyMesh.loadFromAsset("../assets/monkey_smooth.mesh");

	Mesh lostEmpire{};
	lostEmpire.loadFromAsset("../assets/lost_empire.mesh");

	//no vertex normals for now
	upload_mesh(triangleMesh);
	upload_mesh(monkeyMesh);
	upload_mesh(lostEmpire);

	//triangle and monkey are copied in the map
	_meshes["monkey"] = monkeyMesh;
	_meshes["triangle"] = triangleMesh;
	_meshes["empire"] = lostEmpire;
}

void VulkanEngine::upload_mesh(Mesh& mesh)
{
	const size_t bufferSize = mesh._vertices.size() * sizeof(Vertex);
	//allocate staging buffer on cpu
	VkBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;
	stagingBufferInfo.size = bufferSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	AllocatedBuffer stagingBuffer;

	//allocate the buffer on cpu side
	VK_CHECK(vmaCreateBuffer(_allocator,&stagingBufferInfo,&vmaallocInfo,&stagingBuffer._buffer,&stagingBuffer._allocation,nullptr));

	//copy vertex data
	void* data;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(),mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator,stagingBuffer._allocation);

	//allocate vertex buffer on gpu
	VkBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.pNext = nullptr;
	//this is the total size in bytes of the allocated buffer
	vertexBufferInfo.size = bufferSize;
	//this buffer is going to be used as vertex buffer
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	//let vma lib know that this data should be gpu native
	vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator,&vertexBufferInfo,&vmaallocInfo,&mesh._vertexBuffer._buffer,&mesh._vertexBuffer._allocation,nullptr));

	//add destruction of mesh buffer to the deletion queue
	_mainDeletionQueue.push_function([=](){
		vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
	});

	immediate_submit([=](VkCommandBuffer cmd){
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = bufferSize;
		vkCmdCopyBuffer(cmd,stagingBuffer._buffer,mesh._vertexBuffer._buffer,1,&copy);
	});


	//destroy the staging buffer, copy was done so can be freed immediately
	vmaDestroyBuffer(_allocator,stagingBuffer._buffer,stagingBuffer._allocation);
}

void VulkanEngine::init_scene()
{
	_camPos = { 0.f,-6.f,-10.f};

	RenderObject monkey;
	monkey.mesh = get_mesh("monkey");
	monkey.material = get_material("defaultmesh");
	monkey.transformMatrix = glm::mat4(1.f);

	_renderables.push_back(monkey);

	for (int x = -20; x <= 20; x++)
	{
		for (int y = -20; y < 20; y++)
		{
			RenderObject tri;
			tri.mesh = get_mesh("triangle");
			tri.material = get_material("defaultmesh");
			glm::mat4 translation = glm::translate(glm::mat4(1.f),glm::vec3(x,0,y));
			glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.2,0.2,0.2));
			tri.transformMatrix = translation * scale;

			_renderables.push_back(tri);
		}
	}

	RenderObject map;
	map.mesh = get_mesh("empire");
	map.material = get_material("texturedmesh");
	map.transformMatrix = glm::translate(glm::vec3{5,-10,0});

	_renderables.push_back(map);	

	Material* texturedMat = get_material("texturedmesh");

	//allocate the descriptor set for single-texture to use on the material
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_singleTextureSetLayout;

	vkAllocateDescriptorSets(_device,&allocInfo,&texturedMat->textureSet);

	//create a sampler for the texture
	//use filter nearest to make texture appear blocky, which is what we want
	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VkSampler blockySampler;
	vkCreateSampler(_device,&samplerInfo,nullptr,&blockySampler);

	_mainDeletionQueue.push_function([=](){
		vkDestroySampler(_device,blockySampler,nullptr);
	});

	//write to the descriptor set so that it points to our empire_diffuse texture
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = _loadedTextures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,texturedMat->textureSet,&imageBufferInfo,0);

	vkUpdateDescriptorSets(_device,1,&texture1,0,nullptr);
}

#pragma endregion init

void VulkanEngine::cleanup()
{
	if (_isInitialized)
	{

		//make sur the gpu has stopped doing its things
		vkDeviceWaitIdle(_device);

		_mainDeletionQueue.flush();
		
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		if (enableValidationLayers)
        {
            VulkanInstanceBuilder::DestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
        }

		vkDestroyInstance(_instance, nullptr);

		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::draw()
{
	ImGui::Render();

	// wait until the gpu has finished rendering the last frame, Timeout of 1 seconds.
	VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

	// request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSemaphore,(VkFence)nullptr,&swapchainImageIndex));

	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	// begin the command buffer recording.
	// we will use this command buffer exactly once, so we want to let vulkan know it
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// make a clear color from frame number
	// this will flash with a 120*pi frame period.
	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	// start the main renderpass
	// we will use the clear color from above and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass,_windowExtent,_framebuffers[swapchainImageIndex]);

	VkClearValue clearValues[] = {clearValue,depthClear};

	// connect clear values
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);	

	sort_renderables();

	draw_objects(cmd,_renderables.data(),_renderables.size());

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),cmd);

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
	submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	// submit command buffer to the queue and execute it.
	// renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

	// this will put the image we just rendered into the visible window
	// we want to wait on the _renderSemaphore for that
	// as it is necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::present_info();

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_graphicsQueue,&presentInfo));

	//increases the number of frames drawn
	_frameNumber++;
}

void VulkanEngine::run()
{
	_bQuit = false;
	float speed = 0.05f;
	// main loop
	while (!_bQuit)
	{
		glm::vec3 velocity(0.f);
		// Handle events on queue
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			ImGui_ImplSDL2_ProcessEvent(&e);
			handle_event(e);
		}
		if (_front) velocity.z += speed;
		if (_back)	velocity.z -= speed;
		if (_left)	velocity.x += speed;
		if (_right) velocity.x -= speed;
		_camPos += velocity;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(_window);

		ImGui::NewFrame();
		
		//imgui commands
		ImGui::ShowMetricsWindow();

		draw();
	}
}

void VulkanEngine::handle_event(const SDL_Event& e)
{
	// close the window when user alt-f4s or clicks the X button
	switch (e.type)
	{
	case SDL_QUIT:
		_bQuit = true;
		break;
	case SDL_KEYDOWN:
		handle_key_down(e.key);
		break;
	case SDL_KEYUP:
		handle_key_up(e.key);
		break;
	default:
		break;
	}
}

void VulkanEngine::handle_key_down(const SDL_KeyboardEvent& event)
{
	if (event.repeat == 0)
	{
		if (event.keysym.scancode == SDL_SCANCODE_W)
		{
			_front = true;
		}
		if (event.keysym.scancode == SDL_SCANCODE_S)
		{
			_back = true;
		}
		if (event.keysym.scancode == SDL_SCANCODE_A)
		{
			_left = true;
		}
		if (event.keysym.scancode == SDL_SCANCODE_D)
		{
			_right = true;
		}
	}	
}
	
void VulkanEngine::handle_key_up(const SDL_KeyboardEvent& event)
{
	if (event.repeat == 0)
	{
		if (event.keysym.scancode == SDL_SCANCODE_W)
		{
			_front = false;
		}
		if (event.keysym.scancode == SDL_SCANCODE_S)
		{
			_back = false;
		}
		if (event.keysym.scancode == SDL_SCANCODE_A)
		{
			_left = false;
		}
		if (event.keysym.scancode == SDL_SCANCODE_D)
		{
			_right = false;
		}
	}	
}


Material* VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_materials[name] = mat;
	return &_materials[name];
}

Material* VulkanEngine::get_material(const std::string& name)
{
	auto it = _materials.find(name);
	if (it == _materials.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;	
	}	
}

Mesh* VulkanEngine::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}		
}

void VulkanEngine::sort_renderables()
{
	std::sort(_renderables.begin(),_renderables.end(),[](const RenderObject& a, const RenderObject& b){
		if (a.material<b.material)
		{
			return true;
		}
		if (a.material == b.material)
		{
			return a.mesh<b.mesh;
		}
		return false;
	});	
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count)
{
	//make model view matrix
	//camera view 
	glm::mat4 view = glm::translate(glm::mat4(1.f),_camPos);

	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f),1700.f/900.f,0.1f,200.f);
	projection[1][1] *= -1;

	//fill a GPU camera data struct
	GPUCameraData camData;
	camData.proj = projection;
	camData.view = view;
	camData.viewproj = projection * view;

	//copy it to the buffer
	void* data;
	vmaMapMemory(_allocator,get_current_frame()._cameraBuffer._allocation,&data);

	memcpy(data,&camData,sizeof(GPUCameraData));

	vmaUnmapMemory(_allocator, get_current_frame()._cameraBuffer._allocation);

	float framed = _frameNumber / 120.f;
	_sceneParameters.ambientColor = {sin(framed),0,cos(framed),1};

	char* sceneData;
	vmaMapMemory(_allocator,_sceneParametersBuffer._allocation,(void**)&sceneData);

	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

	memcpy(sceneData,&_sceneParameters,sizeof(GPUSceneData));

	vmaUnmapMemory(_allocator,_sceneParametersBuffer._allocation);

	void* objectData;
	vmaMapMemory(_allocator,get_current_frame()._objectBuffer._allocation,&objectData);
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transformMatrix;
	}
	vmaUnmapMemory(_allocator,get_current_frame()._objectBuffer._allocation);
	


	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesn't match with the already bound one
		if (object.material != lastMaterial)
		{
			vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,object.material->pipeline);
			lastMaterial = object.material;
			
			uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
			//bind the descriptor set when changing pipeline
			vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,object.material->pipelineLayout,0,1,&get_current_frame()._globalDescriptor,1,&uniform_offset);

			//object data descriptor
			vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,object.material->pipelineLayout,1,1,&get_current_frame().__objectDescriptor,0,nullptr);

			if (object.material->textureSet != VK_NULL_HANDLE)
			{
				//texture descriptor
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,object.material->pipelineLayout,2,1,&object.material->textureSet,0,nullptr);
			}
		}

		glm::mat4 model = object.transformMatrix;

		MeshPushConstants constants;
		constants.model = model;

		//upload the mesh to the gpu via push constants
		vkCmdPushConstants(cmd,object.material->pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(MeshPushConstants), &constants);

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != lastMesh)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd,0,1,&object.mesh->_vertexBuffer._buffer,&offset);
			lastMesh = object.mesh;
		}

		//finally the drawcall
		vkCmdDraw(cmd,object.mesh->_vertices.size(),1,0,i);
	}
}

FrameData& VulkanEngine::get_current_frame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize,VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;
	
	AllocatedBuffer buffer;

	VK_CHECK(vmaCreateBuffer(_allocator,&bufferInfo,&vmaallocInfo,&buffer._buffer,&buffer._allocation,nullptr));

	return buffer;
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	//Calculate the required alignement based on minimum device offset alignment
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment -1) & ~(minUboAlignment-1);
	}
	return alignedSize;	
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd = _uploadContext._commandBuffer;

	//begin command buffer recording. we will use this command buffer exactly once before resetting so we tell vulkan that.
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd,&cmdBeginInfo));

	//execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::submit_info(&cmd);

	//submit command buffer to the queue and execute it.
	//_uploadFence will now block until the graphics commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue,1,&submit,_uploadContext._uploadFence));

	vkWaitForFences(_device,1, &_uploadContext._uploadFence,true,9999999999);
	vkResetFences(_device,1,&_uploadContext._uploadFence);

	//reset the command buffers inside the command pool
	vkResetCommandPool(_device,_uploadContext._commandPool,0);
}

void VulkanEngine::load_images()
{
	Texture lostEmpire;

	vkutil::load_image_from_asset(*this,"../assets/lost_empire-RGBA.tx",lostEmpire.image);

	VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image._image,VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device,&imageinfo,nullptr,&lostEmpire.imageView);

	_mainDeletionQueue.push_function([=](){
		vkDestroyImageView(_device,lostEmpire.imageView,nullptr);
	});

	_loadedTextures["empire_diffuse"] = lostEmpire;
}