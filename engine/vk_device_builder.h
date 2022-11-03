#include "vk_types.h"
#include <vector>

class VulkanDeviceBuilder
{

public:

    VulkanDeviceBuilder(const VulkanPhysicalDevice &physicalDevice);

    template<typename T>
    VulkanDeviceBuilder& addPNext(T* structure);

    VulkanDeviceBuilder& build();

    VulkanDevice& value();

private:
    std::vector<VkBaseOutStructure*> _pNextChain;

    const VulkanPhysicalDevice& _physicalDevice;
    VulkanDevice _value;

    void createLogicalDevice();
};


template<typename T>
VulkanDeviceBuilder& VulkanDeviceBuilder::addPNext(T* structure)
{
    _pNextChain.push_back(reinterpret_cast<VkBaseOutStructure*> (structure));
    return *this;
}
