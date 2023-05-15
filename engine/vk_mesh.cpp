#include <iostream>
#include <tiny_obj_loader.h>

#include <vk_mesh.h>
#include <asset_loader.h>
#include <mesh_asset.h>
#include <logger.h>

VertexInputDescription Vertex::get_vertex_description()
{
    VertexInputDescription description;

    //we will have just 1 vertex buffer binding, with a per vertex rate
    VkVertexInputBindingDescription mainBinding{};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    //Position will be stored at location 0
    VkVertexInputAttributeDescription positionAttribute{};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex,position);

    //Normals will be stored at location 1
    VkVertexInputAttributeDescription normalAttribute{};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(Vertex,normal);

    //Color will be stored at location 2
    VkVertexInputAttributeDescription colorAttribute{};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(Vertex,color);

    VkVertexInputAttributeDescription uvAttribute{};
    uvAttribute.binding = 0;
    uvAttribute.location = 3;
    uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttribute.offset = offsetof(Vertex,uv);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(normalAttribute);
    description.attributes.push_back(colorAttribute);
    description.attributes.push_back(uvAttribute);

    return description;
}

bool Mesh::load_from_obj(const char* filename)
{
    //attrib will contain the vertex arrays of the file
    tinyobj::attrib_t attrib;
    //shapes contains the info for each separate objet in the file
    std::vector<tinyobj::shape_t> shapes;
    //materials contains the informations about the material of each shape, not used for now
    std::vector<tinyobj::material_t> materials;

    //error and warning output from the load function
    std::string warn;
    std::string err;

    //load the obj file
    tinyobj::LoadObj(&attrib,&shapes,&materials,&warn,&err,filename,nullptr);

    //make sur to output the warnings to the console, in case there are issues with the file
    if (!warn.empty())
    {
        LOG_WARNING(warn);
    }
    //if error break the mesh loading
    //happens if file not found or malformed
    if (!err.empty())
    {
        LOG_ERROR(err);
        return false;
    }
    
    //loop over shapes
    for(auto& shape : shapes)
    {
        size_t index_offset = 0;
        //loop over face
        for(auto& f : shape.mesh.num_face_vertices)
        {
            int fv = 3;
            for (size_t v = 0; v < fv; v++)
            {
                //access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset+v];

                //vertex position
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
            
                //vertex normal
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
            
                tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

                //copy it into our vertex
                Vertex new_vert;
                new_vert.position.x = vx;
                new_vert.position.y = vy;
                new_vert.position.z = vz;

                new_vert.normal.x = nx;
                new_vert.normal.y = ny;
                new_vert.normal.z = nz;

                //we are setting the vertex color as the vertex normal for display purposes
                new_vert.color = new_vert.normal;
                
                new_vert.uv.x = ux;
                new_vert.uv.y = 1-uy;

                _vertices.push_back(new_vert);
            

            }
            index_offset += fv;
        }
    }
    return true;
}

bool Mesh::loadFromAsset(const char* filename)
{
    assets::AssetFile file;
    bool loaded = assets::loadBinaryFile(filename, file);

    if (!loaded)
    {
        LOG_ERROR("Error when loading mesh {}", filename);
        return false;
    }
    
    assets::MeshInfo meshInfo = assets::readMeshInfo(&file);


    std::vector<char> vertexBuffer;
    std::vector<char> indexBuffer;

    vertexBuffer.resize(meshInfo.vertexBufferSize);
    indexBuffer.resize(meshInfo.indexBufferSize);

    assets::unpackMesh(&meshInfo,file.binaryBlob.data(),file.binaryBlob.size(),vertexBuffer.data(),indexBuffer.data());

    _vertices.clear();
    

    assets::Vertex* unpackedVertices = reinterpret_cast<assets::Vertex*>(vertexBuffer.data());

    _vertices.resize(vertexBuffer.size() / sizeof(assets::Vertex));

    for (size_t i = 0; i < _vertices.size(); i++)
    {
        _vertices[i].position.x = unpackedVertices[i].position[0];
        _vertices[i].position.y = unpackedVertices[i].position[1];
        _vertices[i].position.z = unpackedVertices[i].position[2];
        _vertices[i].normal.x = unpackedVertices[i].normal[0];
        _vertices[i].normal.y = unpackedVertices[i].normal[1];
        _vertices[i].normal.z = unpackedVertices[i].normal[2];
        _vertices[i].uv.x = unpackedVertices[i].uv[0];
        _vertices[i].uv.y = unpackedVertices[i].uv[1];
        _vertices[i].color.x = unpackedVertices[i].color[0];
        _vertices[i].color.y = unpackedVertices[i].color[1];
        _vertices[i].color.z = unpackedVertices[i].color[2];
    }
    

    return true;

}