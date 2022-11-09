#include <vk_descriptors.h>

void DescriptorAllocator::init(VkDevice device)
{
    _device = device;
}

void DescriptorAllocator::cleanup()
{
    //delete every pool held
    for (auto pool : _freePools)
    {
        vkDestroyDescriptorPool(_device,pool,nullptr);
    }
    for(auto pool : _usedPools)
    {
        vkDestroyDescriptorPool(_device,pool,nullptr);
    }
}

VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
{
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(poolSizes.sizes.size());
    for(auto size : poolSizes.sizes)
    {
        sizes.push_back({size.first, uint32_t(size.second * count)});
    }

    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.maxSets = count;
    info.poolSizeCount = (uint32_t)sizes.size();
    info.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device,&info,nullptr,&descriptorPool);
    return descriptorPool;
}

VkDescriptorPool DescriptorAllocator::grabPool()
{
    //there are reusable pools available
    if (_freePools.size() > 0)
    {
        //grab pool from the back of the vector and remove it from there.
        VkDescriptorPool pool = _freePools.back();
        _freePools.pop_back();
        return pool;
    }
    else 
    {
        //no pools available, create a new one
        //arbitrary size of 1000, could create growing pool or different sizes
        return createPool(_device,_descriptorSizes,1000,0);
    }    
}

bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
    //initialize the currentpool handle if it's null
    if (_currentPool == VK_NULL_HANDLE)
    {
        _currentPool = grabPool();
        _usedPools.push_back(_currentPool);        
    }

    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.pSetLayouts = &layout;
    info.descriptorPool = _currentPool;
    info.descriptorSetCount = 1;

    //try to allocate the descriptor set
    VkResult allocResult = vkAllocateDescriptorSets(_device, &info, set);
    bool needReallocate = false;
    switch (allocResult)
    {
    case VK_SUCCESS:
        //all good
        return true;
        break;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        //need to reallocate the pool
        needReallocate = true;
        break;
    default:
        //unrecoverable error
        return false;
        break;
    }

    if (needReallocate)
    {
        //allocate a new pool and retry
        _currentPool = grabPool();
        _usedPools.push_back(_currentPool);

        allocResult = vkAllocateDescriptorSets(_device, &info,set);
        if (allocResult == VK_SUCCESS)
        {
            return true;
        }        
        //if it still fails then the issue is quite big
    }
    return false;    
}


void DescriptorAllocator::resetPools()
{
    //reset all used pool and add them to the free pools
    for(auto pool : _usedPools)
    {
        vkResetDescriptorPool(_device,pool,0);
        _freePools.push_back(pool);
    }
    //clear the used pools, they are already in the free pools
    _usedPools.clear();

    //reset the current pool handle back to null
    _currentPool = VK_NULL_HANDLE;
}