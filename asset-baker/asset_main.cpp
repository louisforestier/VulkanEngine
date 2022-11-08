#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <json.hpp>
#include <filesystem>
#include <asset_loader.h>
#include <texture_asset.h>
#include <mesh_asset.h>
#include <chrono>

namespace fs = std::filesystem;

using namespace assets;


bool convertImage(const fs::path& input, const fs::path& output)
{
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(input.u8string().c_str(),&texWidth,&texHeight,&texChannels,STBI_rgb_alpha);
    if (!pixels)
    {
        std::cout << "Failed to load texture file " << input << std::endl;
        return false;
    }
    int imageSize = texWidth * texHeight * 4;

    TextureInfo info;
    info.textureSize = imageSize;
    info.pixelsize[0] = texWidth;
    info.pixelsize[1] = texHeight;
    info.textureFormat = TextureFormat::RGBA8;
    info.originalFile = input.string();
    AssetFile newImage = packTexture(&info,pixels);

    stbi_image_free(pixels);
    saveBinaryFile(output.string().c_str(),newImage);
    return true;
}

bool extractMeshFromObj(std::vector<tinyobj::shape_t>& shapes, tinyobj::attrib_t& attrib, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
{    
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
                new_vert.position[0] = vx;
                new_vert.position[1] = vy;
                new_vert.position[2] = vz;

                new_vert.normal[0] = nx;
                new_vert.normal[1] = ny;
                new_vert.normal[2] = nz;

                //we are setting the vertex color as the vertex normal for display purposes
                //todo change
                new_vert.color[0] = new_vert.normal[0];
                new_vert.color[1] = new_vert.normal[1];
                new_vert.color[2] = new_vert.normal[2];
                
                new_vert.uv[0] = ux;

                //Attention 1 - uy car repère de vulkan sur les textures changent
                //pourrait faire un if ou ifdef en fonction d'un paramètre ou d'un define pour gérer le cas opengl et le cas vulkan
                new_vert.uv[1] = 1-uy;

                indices.push_back(vertices.size());
                vertices.push_back(new_vert);

            }
            index_offset += fv;
        }
    }
    return true;
}



bool convertMesh(const fs::path& input, const fs::path& output)
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

    auto start  = std::chrono::high_resolution_clock::now();

    //load the obj file
    tinyobj::LoadObj(&attrib,&shapes,&materials,&warn,&err,input.string().c_str(),nullptr);

    auto end = std::chrono::high_resolution_clock::now();

    auto diff = end - start;
    std::cout << "obj took " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms" << std::endl;
    //make sur to output the warnings to the console, in case there are issues with the file
    if (!warn.empty())
    {
        std::cout << "WARN: " << warn << std::endl;
    }
    //if error break the mesh loading
    //happens if file not found or malformed
    if (!err.empty())
    {
        std::cerr << err << std::endl;
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    extractMeshFromObj(shapes,attrib,indices,vertices);


    MeshInfo info;
    info.vertexFormat = VertexFormat::PNCV_F32;
    info.vertexBufferSize = vertices.size() * sizeof(Vertex);
    info.indexBufferSize = indices.size() * sizeof(uint32_t);
    info.indexSize = sizeof(uint32_t);
    info.originalFile = input.string();
    info.bounds = calculateBounds(vertices.data(), vertices.size());

    start  = std::chrono::high_resolution_clock::now();

    AssetFile newMesh = packMesh(&info,(char*)vertices.data(),(char*)indices.data());
    
    end  = std::chrono::high_resolution_clock::now();
    
    diff = end - start;

    std::cout << "compression took " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms" << std::endl;

    saveBinaryFile(output.string().c_str(),newMesh);
    return true;
}

int main(int argc, char const *argv[])
{
    fs::path directory{argv[1]};
    std::cout << "loading asset directory at " << directory << std::endl;

    for (auto &p : fs::directory_iterator(directory))
    {
        std::cout << "File: " << p;
        if (p.path().extension() == ".png") 
        {
            std::cout << "found a texture" << std::endl;
            auto path = p.path();
            path.replace_extension(".tx");
            convertImage(p.path(), path);
        }

        if (p.path().extension() == ".obj") 
        {
            std::cout << "found a mesh" << std::endl;
            auto path = p.path();
            path.replace_extension(".mesh");
            convertMesh(p.path(), path);

        }
        
        
    }
    

    return 0;
}
