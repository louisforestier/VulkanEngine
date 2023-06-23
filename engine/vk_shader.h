#pragma once

#include "vk_types.h"
#include <array>
#include <unordered_map>

struct ShaderModule 
{
    std::vector<uint32_t> _code;
    VkShaderModule _module;
};

namespace vkutil
{
 	//load a shader module from a spir-v file. Returns false if it errors.
	bool load_shader_module(VkDevice device, const std::string& filePath, ShaderModule* outShaderModule);
    uint32_t hash_descriptor_layout_info(VkDescriptorSetLayoutCreateInfo* info);
    class DescriptorLayoutCache;
    class DescriptorAllocator;
}

struct ShaderEffect
{
    struct ReflectionOverrides
    {
        const char* _name;
        VkDescriptorType _type;
    };

    void add_stage(ShaderModule* shaderModule, VkShaderStageFlagBits stage);
    void reflect_layout(VkDevice device, vkutil::DescriptorLayoutCache& descriptorLayoutCache, ReflectionOverrides* overrides, int overrideCount);
    void fill_stages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages);

    VkPipelineLayout _builtLayout;

    struct ReflectedBinding
    {
        uint32_t set;
        uint32_t binding;
        VkDescriptorType type;
    };
    std::unordered_map<std::string, ReflectedBinding> bindings;
    std::array<uint32_t,4> setHashes;
    std::array<VkDescriptorSetLayout,4> setLayouts;

private:
    struct ShaderStage
    {
        ShaderModule* _module;
        VkShaderStageFlagBits stage;
    };
    std::vector<ShaderStage> stages;
};

struct ShaderDescriptorBinder
{
    struct BufferWriteDescriptor
    {
        int dstSet;
        int dstBinding;
        VkDescriptorType type;
        VkDescriptorBufferInfo bufferInfo;
        uint32_t dynamicOffset;
    };

    void bind_buffer(const char* name, VkDescriptorBufferInfo& bufferInfo);
    void bind_dynamic_buffer(const char* name, uint32_t offset, const VkDescriptorBufferInfo& bufferInfo);
    void apply_binds(VkCommandBuffer cmd);
    void build_sets(VkDevice device, vkutil::DescriptorAllocator& allocator);
    void set_shader(ShaderEffect* newShader);

    std::array<VkDescriptorSet, 4> cachedDescriptorSets;

private:
    struct DynOffsets
    {
        std::array<uint32_t, 16> offsets;
        uint32_t count{0};
    };

    std::array<DynOffsets,4> setOffsets;
    ShaderEffect* shaders{nullptr};
    std::vector<BufferWriteDescriptor> bufferWrites; 
};

class ShaderCache 
{
public:
    ShaderModule* get_shader(const std::string& path);
    void init(VkDevice device) {_device = device;};

private:
    VkDevice _device;
    std::unordered_map<std::string, ShaderModule> module_cache;
};