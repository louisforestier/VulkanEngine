#include <vk_descriptors.h>
#include <algorithm>
namespace vkutil
{

    void DescriptorAllocator::init(VkDevice device)
    {
        _device = device;
    }

    void DescriptorAllocator::cleanup()
    {
        // delete every pool held
        for (auto pool : _freePools)
        {
            vkDestroyDescriptorPool(_device, pool, nullptr);
        }
        for (auto pool : _usedPools)
        {
            vkDestroyDescriptorPool(_device, pool, nullptr);
        }
    }

    VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes &poolSizes, int count, VkDescriptorPoolCreateFlags flags)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(poolSizes.sizes.size());
        for (auto size : poolSizes.sizes)
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
        vkCreateDescriptorPool(device, &info, nullptr, &descriptorPool);
        return descriptorPool;
    }

    VkDescriptorPool DescriptorAllocator::grabPool()
    {
        // there are reusable pools available
        if (_freePools.size() > 0)
        {
            // grab pool from the back of the vector and remove it from there.
            VkDescriptorPool pool = _freePools.back();
            _freePools.pop_back();
            return pool;
        }
        else
        {
            // no pools available, create a new one
            // arbitrary size of 1000, could create growing pool or different sizes
            return createPool(_device, _descriptorSizes, 1000, 0);
        }
    }

    bool DescriptorAllocator::allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout)
    {
        // initialize the currentpool handle if it's null
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

        // try to allocate the descriptor set
        VkResult allocResult = vkAllocateDescriptorSets(_device, &info, set);
        bool needReallocate = false;
        switch (allocResult)
        {
        case VK_SUCCESS:
            // all good
            return true;
            break;
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            // need to reallocate the pool
            needReallocate = true;
            break;
        default:
            // unrecoverable error
            return false;
            break;
        }

        if (needReallocate)
        {
            // allocate a new pool and retry
            _currentPool = grabPool();
            _usedPools.push_back(_currentPool);
            info.descriptorPool = _currentPool;

            allocResult = vkAllocateDescriptorSets(_device, &info, set);
            if (allocResult == VK_SUCCESS)
            {
                return true;
            }
            // if it still fails then the issue is quite big
        }
        return false;
    }

    void DescriptorAllocator::resetPools()
    {
        // reset all used pool and add them to the free pools
        for (auto pool : _usedPools)
        {
            vkResetDescriptorPool(_device, pool, 0);
            _freePools.push_back(pool);
        }
        // clear the used pools, they are already in the free pools
        _usedPools.clear();

        // reset the current pool handle back to null
        _currentPool = VK_NULL_HANDLE;
    }

    void DescriptorLayoutCache::init(VkDevice device)
    {
        _device = device;
    }

    void DescriptorLayoutCache::cleanup()
    {
        // delete every descriptor layout held
        for (auto &&pair : _layoutCache)
        {
            vkDestroyDescriptorSetLayout(_device, pair.second, nullptr);
        }
    }

    VkDescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(VkDescriptorSetLayoutCreateInfo *info)
    {
        DescriptorLayoutInfo layoutInfo;
        layoutInfo._bindings.reserve(info->bindingCount);
        bool isSorted = true;
        int lastBinding = -1;

        // copy from the direct info struct into our own
        for (size_t i = 0; i < info->bindingCount; i++)
        {
            layoutInfo._bindings.push_back(info->pBindings[i]);

            // check that the bindings are in strict increasing order
            if (info->pBindings[i].binding > lastBinding)
            {
                lastBinding = info->pBindings[i].binding;
            }
            else
            {
                isSorted = false;
            }
        }
        // sort the bindings if they aren't
        if (!isSorted)
        {
            std::sort(layoutInfo._bindings.begin(), layoutInfo._bindings.end(), [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b)
                      { return a.binding < b.binding; });
        }

        // try to grab from cache
        auto it = _layoutCache.find(layoutInfo);
        if (it != _layoutCache.end())
        {
            return (*it).second;
        }
        else
        {
            // create a new one
            VkDescriptorSetLayout layout;
            vkCreateDescriptorSetLayout(_device, info, nullptr, &layout);

            // add to cache
            _layoutCache[layoutInfo] = layout;
            return layout;
        }
    }

    bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const
    {
        if (other._bindings.size() != _bindings.size())
        {
            return false;
        }
        else
        {
            // compare if each of the bindings are the same. Bindings are sorted so they will match.
            for (size_t i = 0; i < _bindings.size(); i++)
            {
                if (other._bindings[i].binding != _bindings[i].binding)
                {
                    return false;
                }
                if (other._bindings[i].descriptorType != _bindings[i].descriptorType)
                {
                    return false;
                }
                if (other._bindings[i].descriptorCount != _bindings[i].descriptorType)
                {
                    return false;
                }
                if (other._bindings[i].stageFlags != _bindings[i].stageFlags)
                {
                    return false;
                }
            }
            return true;
        }
    }

    size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
    {
        using std::hash;
        using std::size_t;

        size_t result = hash<size_t>()(_bindings.size());
        for (const VkDescriptorSetLayoutBinding &b : _bindings)
        {
            // pack the binding data into a single int64.
            // Not fully correct but will do the job
            size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

            // shuffle the packed binding data and xor it with the main hash
            // should also test hash(a)<<1 + hash(a) + hash(b) for improved hashing function but could overflow because of the addition

            result ^= hash<size_t>()(binding_hash);
        }
        return result;
    }

    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
    {
        DescriptorBuilder builder;

        builder._cache = layoutCache;
        builder._alloc = allocator;
        return builder;
    }

    DescriptorBuilder& DescriptorBuilder::bindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        //create the descriptor binding for the layout
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = binding;

        _bindings.push_back(newBinding);

        //create the descriptor write
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;

        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pBufferInfo  = bufferInfo;
        newWrite.dstBinding = binding;

        _writes.push_back(newWrite);
        return *this;                
    }

    DescriptorBuilder& DescriptorBuilder::bindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        //create the descriptor binding for the layout
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = binding;

        _bindings.push_back(newBinding);

        //create the descriptor write
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;

        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pImageInfo = imageInfo;
        newWrite.dstBinding = binding;

        _writes.push_back(newWrite);
        return *this;                
    }

    bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
    {
        //build layout first
        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;

        info.pBindings = _bindings.data();
        info.bindingCount = _bindings.size();

        layout = _cache->createDescriptorLayout(&info);
        
        //allocate descriptor
        bool success = _alloc->allocate(&set,layout);
        if (!success)
        {
            return false;
        }

        //write descriptor
        for(VkWriteDescriptorSet& w : _writes)
        {
            w.dstSet = set;
        }
        vkUpdateDescriptorSets(_alloc->_device,_writes.size(),_writes.data(),0,nullptr);
        return true;
    }

    bool DescriptorBuilder::build(VkDescriptorSet& set)
    {
        VkDescriptorSetLayout layout;
        return build(set, layout);
    }

} // namespace vkutil
