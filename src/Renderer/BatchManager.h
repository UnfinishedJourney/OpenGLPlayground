#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Batch.h"

class BaseRenderObject;
namespace Scene{
    class Camera;
}
class LODEvaluator;

/**
 * @brief Manages a list of RenderObjects, grouping them into Batches
 *        based on shared (shaderName, materialID).
 */
class BatchManager {
public:
    BatchManager() = default;
    ~BatchManager() = default;

    /**
     * @brief Add a RenderObject to the internal list, marking the batch list as stale.
     */
    void AddRenderObject(const std::shared_ptr<BaseRenderObject>& ro);

    /**
     * @brief Build (or rebuild) the batches from the current objects if needed.
     */
    void BuildBatches();

    /**
     * @brief Clear all objects and batches.
     */
    void Clear();

    /**
     * @return The final set of Batches after building.
     */
    const std::vector<std::shared_ptr<Graphics::Batch>>& GetBatches() const;

    // LOD / culling
    void UpdateLOD(const std::shared_ptr<BaseRenderObject>& ro, size_t newLOD);
    void UpdateLODs(std::shared_ptr<Scene::Camera>& camera, LODEvaluator& lodEvaluator);
    void SetLOD(size_t forcedLOD); // for debug (force all objects to this LOD)
    void CullObject(const std::shared_ptr<BaseRenderObject>& ro);

private:
    std::vector<std::shared_ptr<BaseRenderObject>> m_RenderObjects; // All objects
    std::vector<std::shared_ptr<Graphics::Batch>>            m_Batches;       // Final grouping

    // Lookups: object -> the batch that it belongs to
    std::unordered_map<BaseRenderObject*, std::shared_ptr<Graphics::Batch>> m_ObjToBatch;

    bool m_Built = false;

private:
    std::vector<std::shared_ptr<Graphics::Batch>> BuildBatchesFromObjects(
        const std::vector<std::shared_ptr<BaseRenderObject>>& objs
    );

    std::shared_ptr<Graphics::Batch> FindBatchForObject(const std::shared_ptr<BaseRenderObject>& ro) const;
};