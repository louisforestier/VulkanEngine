#include <mesh_asset.h>
#include <json.hpp>
#include <lz4.h>

assets::VertexFormat parse_format(const std::string &format)
{
    if (format == "PNCV_F32")
    {
        return assets::VertexFormat::PNCV_F32;
    }
    else
    {
        return assets::VertexFormat::Unknown;
    }
}

assets::MeshInfo assets::readMeshInfo(AssetFile *file)
{
    MeshInfo info;
    nlohmann::json meshMetaData = nlohmann::json::parse(file->json);
    std::string formatString = meshMetaData["format"];
    info.vertexFormat = parse_format(formatString);

    std::string compressionString = meshMetaData["compression"];
    info.compressionMode = parse_compression(compressionString);
    info.vertexBufferSize = meshMetaData["vertexBufferSize"];
    info.indexBufferSize = meshMetaData["indexBufferSize"];
    info.indexSize = (uint8_t)meshMetaData["indexSize"];
    info.originalFile = meshMetaData["originalSize"];

    std::vector<float> boundsData;
    boundsData.resize(7);
    boundsData = meshMetaData["bounds"].get<std::vector<float>>();
    info.bounds.origin[0] = boundsData[0];
    info.bounds.origin[1] = boundsData[1];
    info.bounds.origin[2] = boundsData[2];

    info.bounds.radius = boundsData[3];

    info.bounds.extents[0] = boundsData[4];
    info.bounds.extents[1] = boundsData[5];
    info.bounds.extents[2] = boundsData[6];
    return info;
}

void assets::unpackMesh(MeshInfo *info, const char *sourcebuffer, size_t sourceSize, char *vertexBuffer, char *indexBuffer)
{
    // decompressing into temporal vector
    // should try to do streaming decompression directly in the buffers
    std::vector<float> decompressedBuffer;
    decompressedBuffer.resize(info->vertexBufferSize);

    LZ4_decompress_safe(sourcebuffer, decompressedBuffer.data(), static_cast<int>(sourceSize), static_cast<int>(decompressedBuffer.size));

    // copy vertex buffer
    memcpy(vertexBuffer, decompressedBuffer.data(), info->vertexBufferSize);

    // copy index buffer
    memcpy(indexBuffer, decompressedBuffer.data() + info->vertexBufferSize, info->indexBufferSize);
}

assets::AssetFile assets::packMesh(MeshInfo *info, char *vertexBuffer, char *indexBuffer)
{
    nlohmann::json meshMetadata;
    if (info->vertexFormat == VertexFormat::PNCV_F32)
    {
        meshMetadata["format"] = "PNCV_F32";
    }
    meshMetadata["vertexBufferSize"] = info->vertexBufferSize;
    meshMetadata["indexBufferSize"] = info->indexBufferSize;
    meshMetadata["indexSize"] = info->indexSize;
    meshMetadata["originalFile"] = info->originalFile;

    std::vector<float> boundsData;
    boundsData.resize(7);
    boundsData[0] = info->bounds.origin[0];
    boundsData[1] = info->bounds.origin[1];
    boundsData[2] = info->bounds.origin[2];
    boundsData[3] = info->bounds.radius;
    boundsData[4] = info->bounds.extents[0];
    boundsData[5] = info->bounds.extents[1];
    boundsData[6] = info->bounds.extents[2];

    AssetFile file;
    file.type[0] = 'M';
    file.type[1] = 'E';
    file.type[2] = 'S';
    file.type[3] = 'H';
    file.version = 1;

    size_t fullSize = info->vertexBufferSize + info->indexBufferSize;

    std::vector<char> mergedBuffer;
    mergedBuffer.resize(fullSize);

    // copy vertex buffer
    memcpy(mergedBuffer.data(), vertexBuffer, info->vertexBufferSize);

    // copy index buffer
    memcpy(mergedBuffer.data() + info->vertexBufferSize, indexBuffer, info->indexBufferSize);

    // compress buffer into blob
    // set buffer size to maximum worst case compression size
    int compressStaging = LZ4_compressBound(static_cast<int>(fullSize));
    file.binaryBlob.resize(compressStaging);

    // set size to actual compressed data
    int compressedSize = LZ4_compress_default(mergedBuffer.data(), file.binaryBlob.data(), static_cast<int>(mergedBuffer.size()), static_cast<int>(compressStaging));
    file.binaryBlob.resize(compressedSize);

    meshMetadata["compression"] = "LZ4";

    file.json = meshMetadata.dump();

    return file;
}

assets::MeshBounds assets::calculateBounds(Vertex *vertices, size_t count)
{
    MeshBounds bounds;

    float min[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float max[3] = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

    for (size_t i = 0; i < count; i++)
    {
        min[0] = std::min(min[0], vertices[i].position[0]);
        min[1] = std::min(min[1], vertices[i].position[1]);
        min[2] = std::min(min[2], vertices[i].position[2]);

        max[0] = std::max(max[0], vertices[i].position[0]);
        max[1] = std::max(max[1], vertices[i].position[1]);
        max[2] = std::max(max[2], vertices[i].position[2]);
    }

    bounds.extents[0] = (max[0] - min[0]) / 2.0f;
    bounds.extents[1] = (max[1] - min[1]) / 2.0f;
    bounds.extents[2] = (max[2] - min[2]) / 2.0f;

    bounds.origin[0] = bounds.extents[0] + min[0];
    bounds.origin[1] = bounds.extents[1] + min[1];
    bounds.origin[2] = bounds.extents[2] + min[2];

    // go through the vertices again to calculate the exact bounding sphere radius
    float r2 = 0;
    for (int i = 0; i < count; i++)
    {
        float offset[3];
        offset[0] = vertices[i].position[0] - bounds.origin[0];
        offset[1] = vertices[i].position[1] - bounds.origin[1];
        offset[2] = vertices[i].position[2] - bounds.origin[2];

        // pithagoras
        float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
        r2 = std::max(r2, distance);
    }

    bounds.radius = std::sqrt(r2);

    return bounds;
}