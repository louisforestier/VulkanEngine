// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

// we will add our main reusable types here

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include <iostream>
#include <logger.h>
#include <cassert>

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
            LOG_ERROR("Detected Vulkan error: {}.", err);           \
			abort();                                                \
		}                                                           \
	} while (0)


struct AllocatedBuffer
{
    VkBuffer _buffer;
    VmaAllocation _allocation;
};

struct AllocatedImage
{
    VkImage _image;
    VmaAllocation _allocation;
};

struct VulkanInstance
{
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    bool _enableValidationLayers;
    std::vector<const char *> _layers;
};

struct VulkanPhysicalDevice
{
    VkPhysicalDevice _device;
    VkSampleCountFlagBits _msaaSamples;
    uint32_t _graphicsQueueFamily;
    uint32_t _presentQueueFamily;
    VkPhysicalDeviceProperties _properties;
    VkPhysicalDeviceFeatures _features;
    std::vector<const char *> _extensions;
    bool _enableValidationLayers;
    std::vector<const char *> _layers;
};

struct VulkanDevice
{
    VkDevice _device;
};

struct VulkanSwapchain
{
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imagesviews;
    VkFormat _imageFormat;
};

enum class MeshPassType : uint8_t {
	None = 0,
	Forward = 1,
	Transparency = 2,
	DirectionalShadow = 3
};
