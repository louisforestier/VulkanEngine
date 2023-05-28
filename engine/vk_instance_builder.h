#include <iostream>
#include <vector>
#include "vk_types.h"

class VulkanInstanceBuilder {

public:


    VulkanInstanceBuilder& setApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch);

    VulkanInstanceBuilder& setAppName(const char* name);

    VulkanInstanceBuilder& setEngineName(const char* name);

    VulkanInstanceBuilder& enableValidationLayers(bool enable = true);

    VulkanInstanceBuilder& addExtension(const char * requiredExtension);

    VulkanInstanceBuilder& addExtensions(std::vector<const char*> requiredExtensions);
    
    VulkanInstanceBuilder& build();
    
    const VulkanInstance& value();

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator);

private:
    VulkanInstance _value; 
    const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

    uint32_t _apiVersion;
    bool _enableValidationLayers;
    const char* _appName = nullptr;
    const char* _engineName = nullptr;
    std::vector<const char*> _extensions;

    bool checkValidationLayerSupport();
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void createInstance();

    void setUpDebugMessenger();

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback);


};