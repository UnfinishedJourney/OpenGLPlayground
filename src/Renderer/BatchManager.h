#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Batch.h"

class BaseRenderObject;
namespace Scene {
    class Camera;
}
class LODEvaluator;

class BatchManager {
public:
    BatchManager() = default;
    ~BatchManager() = default;

    // Adds a render object and marks the batches as stale.
    void AddRenderObject(const std::shared_ptr<BaseRenderObject>& ro);

    // Builds (or rebuilds) batches if needed.
    void BuildBatches();

    // Clears all render objects and batches.
    void Clear();

    // Returns the final set of batches.
    const std::vector<std::shared_ptr<renderer::Batch>>& GetBatches() const;

    // LOD and culling updates.
    void UpdateLOD(const std::shared_ptr<BaseRenderObject>& ro, size_t newLOD);
    void UpdateLODs(std::shared_ptr<Scene::Camera>& camera, LODEvaluator& lodEvaluator);
    void SetLOD(size_t forcedLOD);
    void CullObject(const std::shared_ptr<BaseRenderObject>& ro);

private:
    std::vector<std::shared_ptr<BaseRenderObject>> renderObjects_;
    std::vector<std::shared_ptr<renderer::Batch>> batches_;
    std::unordered_map<BaseRenderObject*, std::shared_ptr<renderer::Batch>> objToBatch_;
    bool built_ = false;

    std::vector<std::shared_ptr<renderer::Batch>> BuildBatchesFromObjects(
        const std::vector<std::shared_ptr<BaseRenderObject>>& objs);
    std::shared_ptr<renderer::Batch> FindBatchForObject(const std::shared_ptr<BaseRenderObject>& ro) const;
};