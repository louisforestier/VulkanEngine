#include <optional>
#include "vk_types.h"

class VulkanDeviceSelector
{
public:
    VulkanDeviceSelector(VulkanInstance& instance, VkSurfaceKHR surface);

    VulkanDeviceSelector& setApiVersion(uint32_t major, uint32_t minor, uint32_t patch);

    VulkanDeviceSelector& addExtension(const char* requiredExtension);

    VulkanDeviceSelector& addExtensions(std::vector<const char*> requiredExtensions);

    VulkanDeviceSelector& select(); 

    const VulkanPhysicalDevice& value();

private:
    VkInstance _instance;
    VkSurfaceKHR _surface;
    VulkanPhysicalDevice _value;

    uint32_t _apiVersion;

    std::vector<const char *> _deviceExtensions;
    std::vector<const char *> _layers;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice device);

    void pickPhysicalDevice();

    int rateDeviceSuitability(VkPhysicalDevice device);

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

};