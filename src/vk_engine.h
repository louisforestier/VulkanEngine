// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>
#include <string>


#include <vk_mesh.h>
#include <vk_textures.h>
#include <vk_types.h>

#include <glm/glm.hpp>

struct SDL_KeyboardEvent;

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 model;
};


struct Material
{
	VkDescriptorSet textureSet{VK_NULL_HANDLE}; //texture defaulted to null
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject
{
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct GPUSceneData{
	glm::vec4 fogColor; //w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused but needed for alignement
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};


struct FrameData
{
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	//buffer that holds a single GPUCameraData to use when rendering
	AllocatedBuffer _cameraBuffer;
	VkDescriptorSet _globalDescriptor;

	AllocatedBuffer _objectBuffer;
	VkDescriptorSet __objectDescriptor;
};

struct GPUObjectData
{
	glm::mat4 modelMatrix;
};

struct UploadContext
{
	VkFence _uploadFence;
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
};




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

//number of frames to overlap when rendering
//2 or 3 at most, 1 to disable double buffering
constexpr unsigned int FRAME_OVERLAP = 2;

struct Texture;

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

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	FrameData _frames[FRAME_OVERLAP];

	DeletionQueue _mainDeletionQueue;

	VmaAllocator _allocator;

	VkImageView _depthImageView;
	AllocatedImage _depthImage;

	VkFormat _depthFormat;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorPool _descriptorPool;

	VkDescriptorSetLayout _objectSetLayout;
	
	VkDescriptorSetLayout _singleTextureSetLayout;

	VkPhysicalDeviceProperties _gpuProperties;

	//default array of renderable objects
	std::vector<RenderObject> _renderables;

	std::unordered_map<std::string,Material> _materials;
	std::unordered_map<std::string,Mesh> _meshes;

	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParametersBuffer;

	UploadContext _uploadContext;

	std::unordered_map<std::string,Texture> _loadedTextures;

	glm::vec3 _camPos;
	bool _bQuit;
	bool _front{};
	bool _back{};
	bool _left{};
	bool _right{};

	AllocatedBuffer create_buffer(size_t allocSize,VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

private:

	void init_vulkan();

	void init_swapchain();

	void init_commands();

	void init_default_renderpass();

	void init_framebuffers();

	void init_sync_structures();

	void init_scene();

	//load a shader module from a spir-v file. Returns false if it errors.
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

	void init_descriptors();

	void init_pipelines();

	void load_meshes();

	void upload_mesh(Mesh& mesh);

	void load_images();

	//create material and add it to the map
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it can't be found
	Material* get_material(const std::string& name);

	//return nullptr if it can't be found
	Mesh* get_mesh(const std::string& name);

	FrameData& get_current_frame();

	size_t pad_uniform_buffer_size(size_t originalSize);

	void sort_renderables();	

	//draw function
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);

	void handle_input();

	void handle_key_down(const SDL_KeyboardEvent& event);
	
	void handle_key_up(const SDL_KeyboardEvent& event);
};
