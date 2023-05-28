#include <iostream>
#include <set>
#include <map>
#include <vk_device_selector.h>
#include <logger.h>

VulkanDeviceSelector::VulkanDeviceSelector(VulkanInstance& instance, VkSurfaceKHR surface)
: _instance(instance._instance), _surface(surface) 
{
    _surface = surface;
    _value._enableValidationLayers = instance._enableValidationLayers;
    _value._layers = instance._layers;
}


VulkanDeviceSelector& VulkanDeviceSelector::setApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch)
{
    _apiVersion = VK_MAKE_API_VERSION(variant,major,minor,patch);
    return *this;
}

VulkanDeviceSelector& VulkanDeviceSelector::addExtension(const char* requiredExtension)
{
    if (requiredExtension != nullptr)
    {
        _deviceExtensions.push_back(requiredExtension);
    }
    return *this;
}

VulkanDeviceSelector& VulkanDeviceSelector::addExtensions(std::vector<const char*> requiredExtensions)
{
    _deviceExtensions.insert(_deviceExtensions.end(),requiredExtensions.begin(),requiredExtensions.end());
    return *this;
}

VulkanDeviceSelector& VulkanDeviceSelector::select()
{
    pickPhysicalDevice();
    if(_value._device != nullptr)
    {
        _value._msaaSamples = getMaxUsableSampleCount(_value._device);
        QueueFamilyIndices indices = findQueueFamilies(_value._device);
        _value._graphicsQueueFamily = indices.graphicsFamily.value();
        _value._presentQueueFamily = indices.presentFamily.value();
        vkGetPhysicalDeviceProperties(_value._device,&_value._properties);
        vkGetPhysicalDeviceFeatures(_value._device,&_value._features);
        _value._extensions = _deviceExtensions;
    }
    return *this;
}

const VulkanPhysicalDevice& VulkanDeviceSelector::value()
{
    return _value;
}


VkSampleCountFlagBits VulkanDeviceSelector::getMaxUsableSampleCount(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanDeviceSelector::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("GPU does not support Vulkan!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto &device : devices)
    {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        _value._device = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("No GPU compatible!");
    }

    if (_value._device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("No GPU compatible!");
    }
}

int VulkanDeviceSelector::rateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int score = 0;
    if (_apiVersion > deviceProperties.apiVersion || !isDeviceSuitable(device))
    {
        return score;
    }    
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }
    else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
    {
        score += 500;
    }
    else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
    {
        score -= 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;
    if (deviceFeatures.geometryShader)
    {
        score += 100;
    }
    score += getMaxUsableSampleCount(device);
    LOG_INFO("Device name: {} - Score= {}", deviceProperties.deviceName,score);
    return score;
}

bool VulkanDeviceSelector::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device,&supportedFeatures);

    return (indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy);
}

VulkanDeviceSelector::QueueFamilyIndices VulkanDeviceSelector::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}


bool VulkanDeviceSelector::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    if (_surface != VK_NULL_HANDLE)
    {
        _deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    
    std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}


VulkanDeviceSelector::SwapChainSupportDetails VulkanDeviceSelector::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr));

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}
