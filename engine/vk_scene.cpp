#include "vk_scene.h"

void RenderScene::buildInstances(const std::vector<RenderObject>& renderObjects)
{
    _batches.clear();
    _transformMatrices.clear();
    _instanceData.clear();
    _instances.clear();
    size_t count = renderObjects.size();
    _batches.resize(count);
    _transformMatrices.resize(count);
    _instanceData.resize(count);
    for (size_t i = 0 ; i < count ; i++)
    {
        const RenderObject* object = &renderObjects.at(i);
        _batches[i].object = object;
        _instanceData[i] = i;
        uint64_t materialHash = std::hash<void*>()(object->material) & UINT32_MAX;
        uint64_t meshHash = std::hash<void*>()(object->mesh) & UINT32_MAX;
        _batches[i].sortKey = materialHash << 32 | meshHash;
        _transformMatrices[i] = object->transformMatrix;
    }

    std::sort(_batches.begin(), _batches.end(), [](const RenderBatch& a, const RenderBatch& b){
        return a.sortKey < b.sortKey;
    });

    _instances.reserve(count / 3);

    InstanceBatch newBatch;
    newBatch.first = 0;
    newBatch.count = 0;
    newBatch.material = _batches[0].object->material;
    newBatch.mesh = _batches[0].object->mesh;
    
    _instances.push_back(newBatch);

    for (int i = 0 ; i < count ; i++)
    {
        const RenderObject *object = _batches[i].object;

        if(object->mesh == _instances.back().mesh && object->material == _instances.back().material)
            _instances.back().count++;
        else
        {
            newBatch.first = i;
            newBatch.count = 1;
            newBatch.material = object->material;
            newBatch.mesh = object->mesh;
            _instances.push_back(newBatch);
        }
    }

}
