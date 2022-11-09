#pragma once

#include <vk_types.h>
#include <vector>
#include <unordered_map>

class DescriptorAllocator
{
public:
    DescriptorAllocator() = default;
    ~DescriptorAllocator() = default;

    struct PoolSizes
    {
        //reasonable default value
        //could be tweaked to the project to optimize allocation
        //could use some config file
        std::vector<std::pair<VkDescriptorType,float>> sizes =
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f}
        };
    };

    void resetPools();

    bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

    void init(VkDevice device);

    void cleanup();

    VkDevice _device;

private:

    VkDescriptorPool grabPool();

    VkDescriptorPool _currentPool{VK_NULL_HANDLE};

    PoolSizes _descriptorSizes;
    std::vector<VkDescriptorPool> _usedPools;
    std::vector<VkDescriptorPool> _freePools;   
};


class DescriptorLayoutCache
{
public:
    void init(VkDevice device);
    void cleanup();

    VkDescriptorSetLayout createDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

    struct DescriptorLayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> _bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;

        size_t hash() const;        
    };

private:
    
    struct DescriptorLayoutHash
    {
        std::size_t operator()(const DescriptorLayoutInfo& k) const{
            return k.hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo,VkDescriptorSetLayout,DescriptorLayoutHash> _layoutCache;
    VkDevice _device;
};