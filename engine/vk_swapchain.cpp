#include "vk_swapchain.h"
#include "vk_initializers.h"

VulkanSwapchainBuilder::VulkanSwapchainBuilder(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueue, uint32_t presentQueue)
: _physicalDevice(physicalDevice)
, _device(device)
, _surface(surface)
, _graphicsQueueFamily(graphicsQueue)
, _presentQueueFamily(presentQueue)
{}

VulkanSwapchainBuilder& VulkanSwapchainBuilder::setPresentMode(VkPresentModeKHR presentMode)
{
    _presentMode = presentMode;
    return *this;
}

VulkanSwapchainBuilder& VulkanSwapchainBuilder::setExtent(uint32_t width, uint32_t height)
{
    _width = width;
    _height = height;
    return *this;
}

VulkanSwapchainBuilder& VulkanSwapchainBuilder::build()
{
    createSwapChain();
    return *this;
}


VulkanSwapchain& VulkanSwapchainBuilder::value()
{
    return _value;
}

VkSurfaceFormatKHR VulkanSwapchainBuilder::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}


void VulkanSwapchainBuilder::createSwapChain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats;
    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, formats.data());
    }


    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }
    VkExtent2D extent = {_width,_height};
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    uint32_t queueFamilyIndices[] = {_graphicsQueueFamily,_presentQueueFamily};

    if (_graphicsQueueFamily != _presentQueueFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = _presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_value._swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("échec de la création de la swap chain!");
    }

    vkGetSwapchainImagesKHR(_device, _value._swapchain, &imageCount, nullptr);
    _value._images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _value._swapchain, &imageCount, _value._images.data());
    _value._imageFormat = surfaceFormat.format;

    _value._imagesviews.resize(_value._images.size());

    for (size_t i = 0; i < _value._images.size(); i++)
    {
        VkImageViewCreateInfo info = vkinit::imageview_create_info(_value._imageFormat,_value._images[i],VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(_device,&info,nullptr,&_value._imagesviews[i]));
    }
}
