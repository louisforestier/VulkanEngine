#pragma once

#include "vk_types.h"
#include "glm/ext/matrix_transform.hpp"

struct Mesh;
struct Material;
class Transform;

struct RenderObject
{
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

struct RenderBatch
{
    const RenderObject* object;
    uint64_t sortKey;
    uint64_t objectIndex;
};

struct InstanceBatch 
{
    Mesh* mesh;
    Material* material;
    uint64_t first;
    uint64_t count;
};


class RenderScene {
private:
    std::vector<RenderBatch> _batches;
    std::vector<InstanceBatch> _instances;
    std::vector<glm::mat4> _transformMatrices;
    std::vector<uint32_t> _instanceData;
public:
    void buildInstances(const std::vector<RenderObject>& renderObjects);
    std::vector<glm::mat4>& getAllTransforms(){return _transformMatrices;}
    std::vector<InstanceBatch>& getAllInstances(){return _instances;}
    std::vector<uint32_t>& getInstanceData(){return _instanceData;}
};