#pragma once

#include "asset_loader.h"

namespace assets
{

    struct Vertex
    {
        float position[3];
        float normal[3];
        float color[3];
        float uv[2];
    };
    

	enum class VertexFormat : uint32_t
	{
		Unknown = 0,
		PNCV_F32
	};

    struct MeshBounds
    {
        float origin[3];
        float radius;
        float extents[3];
    };
    

    struct MeshInfo
    {
        uint64_t vertexBufferSize;
        uint64_t indexBufferSize;
        MeshBounds bounds;
        VertexFormat vertexFormat;
        char indexSize;
        CompressionMode compressionMode;
        std::string originalFile;
    };
    
    //parse the texture metadata from an asset file
    MeshInfo readMeshInfo(AssetFile* file);

    void unpackMesh(MeshInfo* info, const char* sourcebuffer, size_t sourceSize, char* vertexBuffer, char* indexBuffer);

    AssetFile packMesh(MeshInfo* info, char* vertexBuffer, char* indexBuffer);

    MeshBounds calculateBounds(Vertex* vertices, size_t count);

} // namespace assets
