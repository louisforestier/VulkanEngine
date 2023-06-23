#pragma once

#include <vk_types.h>
#include <memory>
#include <array>

#include <material_asset.h>

struct ShaderEffect;

struct ShaderPass
{
    ShaderEffect* effect{nullptr};
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};

template <typename T>
struct PerPassData
{
    T& operator[](MeshPassType pass)
    {
        switch(pass)
        {
            case MeshPassType::Forward:
                return data[0];
            case MeshPassType::Transparency:
                return data[1];
            case MeshPassType::DirectionalShadow:
                return data[2];
        }
        assert(false);
        return data[0];
    }

    void clear(T&& val)
    {
        for (int i = 0 ; i < 3 ; i++)
            data[i] = val;
    }

private:
    std::array<T,3> data;
};

struct ShaderParameters{};

struct EffectTemplate
{
    PerPassData<ShaderPass*> passShaders;
    ShaderParameters* defaultParameters;
    assets::TransparencyMode transparency;
};

struct SampledTexture
{
    VkSampler sampler;
    VkImageView view;
};

struct Material
{
    EffectTemplate* original;
    PerPassData<VkDescriptorSet> passSets;
    std::vector<SampledTexture> textures;
    ShaderParameters* parameters;
};

struct MaterialData
{
    std::vector<SampledTexture> textures;
    ShaderParameters* parameters;
    std::string baseTemplate;
};

class MaterialSystem
{
public:

private:
    std::unique_ptr<ShaderEffect> build_effect(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
};