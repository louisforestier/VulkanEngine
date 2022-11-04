#pragma once

#include "asset_loader.h"

namespace assets
{
	enum class TextureFormat : uint32_t
	{
		Unknown = 0,
		RGBA8
	};

    struct TextureInfo
    {
        uint64_t textureSize;
        TextureFormat textureFormat;
        CompressionMode compressionMode;
        uint32_t pixelsize[3];
        std::string originalFile;
    };
    
    //parse the texture metadata from an asset file
    TextureInfo readTextureInfo(AssetFile* file);

    void unpackTexture(TextureInfo* info, const char* sourcebuffer, size_t sourceSize, char* destination);

    AssetFile packTexture(TextureInfo* info, void* pixelData);

} // namespace assets
