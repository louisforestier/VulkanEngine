#include <vk_device_builder.h>
#include <set>
#include <stdexcept>

VulkanDeviceBuilder::VulkanDeviceBuilder(const VulkanPhysicalDevice& physicalDevice)
: _physicalDevice(physicalDevice)
{}

VulkanDeviceBuilder& VulkanDeviceBuilder::build()
{
    createLogicalDevice();
    return *this;
}

VulkanDevice& VulkanDeviceBuilder::value()
{
    return _value;
}

void VulkanDeviceBuilder::createLogicalDevice()
{

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {_physicalDevice._graphicsQueueFamily,_physicalDevice._presentQueueFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &_physicalDevice._features;
	if (_pNextChain.size () > 0) 
    {
        for (size_t i = 0; i < _pNextChain.size () - 1; i++) {
            _pNextChain.at (i)->pNext = _pNextChain.at (i + 1);
        }
    	createInfo.pNext = _pNextChain.at (0);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_physicalDevice._extensions.size());
    createInfo.ppEnabledExtensionNames = _physicalDevice._extensions.data();

    if (_physicalDevice._enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_physicalDevice._layers.size());
        createInfo.ppEnabledLayerNames = _physicalDevice._layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(_physicalDevice._device, &createInfo, nullptr, &_value._device) != VK_SUCCESS)
    {
        throw std::runtime_error("échec lors de la création d'un logical device");
    }
}
