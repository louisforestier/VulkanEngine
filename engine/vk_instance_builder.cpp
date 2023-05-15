#include "vk_instance_builder.h"
#include <cstring>

#include <logger.h>

VulkanInstanceBuilder& VulkanInstanceBuilder::setApiVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    _apiVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::setAppName(const char *name)
{
    if (name != nullptr)
        _appName = name;
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::setEngineName(const char *name)
{
    if (name != nullptr)
        _engineName = name;
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::enableValidationLayers(bool enable)
{
    _enableValidationLayers = enable;
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::addExtension(const char *requiredExtension)
{
    if (requiredExtension != nullptr)
    {
        _extensions.push_back(requiredExtension);
    }
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::addExtensions(std::vector<const char *> requiredExtensions)
{
    _extensions.insert(_extensions.end(), requiredExtensions.begin(), requiredExtensions.end());
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::build()
{
    createInstance();
    setUpDebugMessenger();
    _value._enableValidationLayers = _enableValidationLayers;
    _value._layers = validationLayers;
    return *this;
}

const VulkanInstance& VulkanInstanceBuilder::value() { return _value; }

bool VulkanInstanceBuilder::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstanceBuilder::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    LOG_ERROR("validation layer: {}", pCallbackData->pMessage);

    if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        LOG_ERROR("A specification violation or potential error has occurred.");
    }
    else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        LOG_ERROR("Potentially non-optimal use of Vulkan.");
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_ERROR("validation layer: {}", pCallbackData->pMessage);
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        return VK_TRUE;
    return VK_FALSE;
}

void VulkanInstanceBuilder::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanInstanceBuilder::createInstance()
{
    if (_enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validations layers required but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = _appName != nullptr ? _appName : "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = _engineName != nullptr ? _engineName : "";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (_enableValidationLayers)
    {
        _extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
    createInfo.ppEnabledExtensionNames = _extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_value._instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance!");

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> vkExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data());

    // std::cout << "Extensions disponibles : \n";

    // for (const auto &extension : vkExtensions)
    // {
    //     std::cout << '\t' << extension.extensionName << '\n';
    // }

}

void VulkanInstanceBuilder::setUpDebugMessenger()
{
    if (!_enableValidationLayers)
        return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(_value._instance, &createInfo, nullptr, &_value._debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create debug messenger!");
    }
}


VkResult VulkanInstanceBuilder::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstanceBuilder::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}
