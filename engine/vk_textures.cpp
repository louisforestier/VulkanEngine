#include <vk_textures.h>
#include <vk_engine.h>

#include <iostream>

#include <vk_initializers.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <tracy/Tracy.hpp>

#include <asset_loader.h>
#include <texture_asset.h>
#include <logger.h>

bool vkutil::load_image_from_file(VulkanEngine& engine, const std::string& filename, AllocatedImage& outImage)
{
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(filename.c_str(),&texWidth,&texHeight,&texChannels,STBI_rgb_alpha);
    if (!pixels)
    {
        LOG_ERROR("Failed to load texture file {}", filename);
        return false;
    }

    void* pixel_ptr = pixels;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    //format r8g8b8a8 matches exactly with the pixels load from stb_image lib
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

    //allocate temporary buffer for holding texture data to upload
    AllocatedBuffer stagingBuffer = engine.create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VMA_MEMORY_USAGE_CPU_ONLY);

    //copy data to buffer
    void* data;
    vmaMapMemory(engine._allocator, stagingBuffer._allocation, &data);

    memcpy(data,pixel_ptr,static_cast<size_t>(imageSize));

    vmaUnmapMemory(engine._allocator, stagingBuffer._allocation);

    //we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    VkImageCreateInfo dimg_info = vkinit::image_create_info(imageFormat,VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

    AllocatedImage newImage;

    VmaAllocationCreateInfo dimg_allocinfo{};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    //allocate and create the image
    vmaCreateImage(engine._allocator,&dimg_info,&dimg_allocinfo,&newImage._image,&newImage._allocation,nullptr);

    engine.immediate_submit([&](VkCommandBuffer cmd){
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrierToTransfer{};
        imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToTransfer.image = newImage._image;
        imageBarrierToTransfer.subresourceRange = range;
        imageBarrierToTransfer.srcAccessMask = 0;
        imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        //barrier the image into the transfer receive layout
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&imageBarrierToTransfer);

        VkBufferImageCopy copy{};
        copy.bufferOffset = 0;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.mipLevel = 0;
        copy.imageSubresource.baseArrayLayer = 0;
        copy.imageSubresource.layerCount = 1;
        copy.imageExtent = imageExtent;

        //copy buffer into image
        vkCmdCopyBufferToImage(cmd,stagingBuffer._buffer,newImage._image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&copy);
    
        VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
        imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        //barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&imageBarrierToReadable);
    });
    VmaAllocator& allocator = engine._allocator;
    engine._mainDeletionQueue.push_function([=](){
        vmaDestroyImage(allocator,newImage._image,newImage._allocation);
    });

    vmaDestroyBuffer(engine._allocator,stagingBuffer._buffer,stagingBuffer._allocation);

    LOG_SUCCESS("Texture loaded successfully {}.", filename);

    outImage=newImage;
    return true;
}

AllocatedImage uploadImage(int texWidth, int texHeight, VkFormat textureFormat, VulkanEngine& engine, AllocatedBuffer& stagingBuffer)
{
    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    VkImageCreateInfo dimg_info = vkinit::image_create_info(textureFormat,VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

    AllocatedImage newImage;

    VmaAllocationCreateInfo dimg_allocinfo{};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    //allocate and create the image
    vmaCreateImage(engine._allocator,&dimg_info,&dimg_allocinfo,&newImage._image,&newImage._allocation,nullptr);

    engine.immediate_submit([&](VkCommandBuffer cmd){
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrierToTransfer{};
        imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToTransfer.image = newImage._image;
        imageBarrierToTransfer.subresourceRange = range;
        imageBarrierToTransfer.srcAccessMask = 0;
        imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        //barrier the image into the transfer receive layout
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&imageBarrierToTransfer);

        VkBufferImageCopy copy{};
        copy.bufferOffset = 0;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.mipLevel = 0;
        copy.imageSubresource.baseArrayLayer = 0;
        copy.imageSubresource.layerCount = 1;
        copy.imageExtent = imageExtent;

        //copy buffer into image
        vkCmdCopyBufferToImage(cmd,stagingBuffer._buffer,newImage._image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&copy);
    
        VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
        imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        //barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&imageBarrierToReadable);
    });
    VmaAllocator& allocator = engine._allocator;
    engine._mainDeletionQueue.push_function([=](){
        vmaDestroyImage(allocator,newImage._image,newImage._allocation);
    });
    return newImage;
}

bool vkutil::load_image_from_asset(VulkanEngine& engine, const std::string& filename, AllocatedImage& outImage)
{
    assets::AssetFile file;
    bool loaded = assets::loadBinaryFile(filename, file);

    if (!loaded)
    {
        LOG_ERROR("Error when loading image {}", filename);
        return false;
    }
    
    assets::TextureInfo textureInfo = assets::readTextureInfo(&file);

    VkDeviceSize textureSize = textureInfo.textureSize;
    VkFormat textureFormat;
    switch (textureInfo.textureFormat)
    {
    case assets::TextureFormat::RGBA8:
        textureFormat = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    
    default:
        return false;
        break;
    }

    AllocatedBuffer stagingBuffer = engine.create_buffer(textureSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_UNKNOWN, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    void* data;
    vmaMapMemory(engine._allocator,stagingBuffer._allocation,&data);

    {
        ZoneScopedNC("Unpack texture", tracy::Color::Magenta);
        assets::unpackTexture(&textureInfo,file.binaryBlob.data(),file.binaryBlob.size(),(char*) data);
    }
    
    vmaUnmapMemory(engine._allocator,stagingBuffer._allocation);
    outImage = uploadImage(textureInfo.pixelsize[0],textureInfo.pixelsize[1],textureFormat,engine,stagingBuffer);

    vmaDestroyBuffer(engine._allocator,stagingBuffer._buffer,stagingBuffer._allocation);
    LOG_SUCCESS("Texture loaded successfully {}.", filename);

    return true;
}

