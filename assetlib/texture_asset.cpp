#include <texture_asset.h>
#include <json.hpp>
#include <lz4.h>

assets::AssetFile assets::packTexture(assets::TextureInfo* info, void* pixelData)
{
    nlohmann::json textureMetadata;
    textureMetadata["format"] ="RGBA8";
    textureMetadata["width"] = info->pixelsize[0];
    textureMetadata["height"] = info->pixelsize[1];
    textureMetadata["buffer_size"] = info->textureSize;
    textureMetadata["original_file"] = info->originalFile;

    //core file header

    AssetFile file;
    file.type[0] = 'T';
    file.type[1] = 'E';
    file.type[2] = 'X';
    file.type[3] = 'I';
    file.version = 1;

    //compress buffer into blob
    //set buffer size to maximum worst case compression size
    int compressStaging = LZ4_compressBound(info->textureSize);
    file.binaryBlob.resize(compressStaging);

    //set size to actual compressed data
    int compressedSize = LZ4_compress_default((const char*)pixelData,file.binaryBlob.data(),info->textureSize,compressStaging);
    file.binaryBlob.resize(compressedSize);

    textureMetadata["compression"] = "LZ4";

    std::string stringified = textureMetadata.dump();
    file.json = stringified;
    return file;
}

assets::TextureFormat parse_format(const std::string& format) {

	if (format == "RGBA8")
	{
		return assets::TextureFormat::RGBA8;
	}
	else {
		return assets::TextureFormat::Unknown;
	}
}

assets::TextureInfo assets::readTextureInfo(AssetFile* file)
{
    TextureInfo info;

    nlohmann::json textureMetadata = nlohmann::json::parse(file->json);
    std::string formatString = textureMetadata["format"];
    info.textureFormat = parse_format(formatString);

    std::string compressionString = textureMetadata["compression"];
    info.compressionMode = parse_compression(compressionString);

    info.pixelsize[0] = textureMetadata["width"];
    info.pixelsize[1] = textureMetadata["height"];
    info.textureSize = textureMetadata["buffer_size"];
    info.originalFile = textureMetadata["original_file"];

    return info;  
}

void assets::unpackTexture(TextureInfo* info, const char* sourcebuffer, size_t sourcesize, char* destination)
{
    if (info->compressionMode == CompressionMode::LZ4)
    {
        LZ4_decompress_safe(sourcebuffer,destination,sourcesize,info->textureSize);
    }
    else
    {
        memcpy(destination,sourcebuffer,sourcesize);
    }
    
    
}