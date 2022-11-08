#pragma once

#include <vk_types.h>

class VulkanEngine;

namespace vkutil
{
    bool load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& outImage);
    bool load_image_from_asset(VulkanEngine& engine, const char* filename, AllocatedImage& outImage);
} // namespace vkutil

struct Texture
{
    AllocatedImage image;
    VkImageView imageView;
};
