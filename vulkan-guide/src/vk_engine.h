// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vector>
#include <deque>
#include <functional>

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()> &&function)
	{
		deletors.push_back(function);
	}

	void flush()
	{
		//reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
		{
			(*it)(); //call the function
		}
		deletors.clear();
		
	}
};

class VulkanEngine {
public:


	bool _isInitialized{ false };
	int _frameNumber {0};

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	VkInstance _instance; //Vulkan context handle
	VkDebugUtilsMessengerEXT _debug_messenger; //Vulkan debug output handle
	VkPhysicalDevice _chosenGPU; //GPU chosen as the default device
	VkDevice _device ;//Vulkan device for commands
	VkSurfaceKHR _surface; // Vulkan window surface

	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat; // Image format expected by the windowing system
	std::vector<VkImage> _swapchainImages; // Array of images from the swapchain
	std::vector<VkImageView> _swapchainImageViews; // Array of image-views from the swapchain

	VkQueue _graphicsQueue; // queue we will submit to
	uint32_t _graphicsQueueFamily; // family of that queue

	VkCommandPool _commandPool; // Pool containing our commands
	VkCommandBuffer _mainCommandBuffer; // the buffer we will record into

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkPipelineLayout _trianglePipelineLayout;
	VkPipeline _trianglePipeline;
	VkPipeline _redTrianglePipeline;

	int _selectedShader{0};

	DeletionQueue _mainDeletionQueue;

private:

	void init_vulkan();

	void init_swapchain();

	void init_commands();

	void init_default_renderpass();

	void init_framebuffers();

	void init_sync_structures();

	//load a shader module from a spir-v file. Returns false if it errors.
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

	void init_pipelines();
};
