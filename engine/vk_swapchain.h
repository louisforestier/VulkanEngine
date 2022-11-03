#include "vk_types.h"

class VulkanSwapchainBuilder {

public:
    VulkanSwapchainBuilder(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueue, uint32_t presentQueue);

    VulkanSwapchainBuilder& setPresentMode(VkPresentModeKHR presentMode);
    VulkanSwapchainBuilder& setExtent(uint32_t width, uint32_t height);
    VulkanSwapchainBuilder& build();
    VulkanSwapchain& value();

private:
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkSurfaceKHR _surface;
    uint32_t _graphicsQueueFamily;
    uint32_t _presentQueueFamily;

    uint32_t _width;
    uint32_t _height;
    VkPresentModeKHR _presentMode;

    VulkanSwapchain _value;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    void createSwapChain();


};