#pragma once

#include "vk_types.h"
#include <array>

struct ShaderModule 
{
    std::vector<uint32_t> _code;
    VkShaderModule _module;
};

namespace vkutil
{
 	//load a shader module from a spir-v file. Returns false if it errors.
	bool load_shader_module(VkDevice device, const std::string& filePath, ShaderModule* outShaderModule);
    class DescriptorLayoutCache;
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

    VkPipelineLayout _builtLayout;

private:
    struct ShaderStage
    {
        ShaderModule* _module;
        VkShaderStageFlagBits stage;
    };
    std::vector<ShaderStage> stages;
    std::array<VkDescriptorSetLayout,4> setLayouts;
};
