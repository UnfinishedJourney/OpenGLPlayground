#pragma once

#include <memory>
#include <vector>
#include "Renderer/Batch.h"
#include "Scene/Camera.h"

class LODEvaluator;

/**
 * @brief BatchManager accumulates RenderObjects and groups them by (Shader, Material, MeshLayout).
 *        Then it builds a list of Batches. Each Batch can do a multi-draw.
 */
class BatchManager
{
public:
    void AddRenderObject(const std::shared_ptr<RenderObject>& ro);
    void BuildBatches();
    void Clear();

    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;

    // LOD updates
    void UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD);
    void UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator);

    // Optional: force setting LOD for all objects (dev tool?)
    void SetLOD(size_t newLOD);

    // Culling
    void CullObject(const std::shared_ptr<RenderObject>& ro);

private:
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
    std::vector<std::shared_ptr<Batch>>        m_AllBatches;
    bool                                       m_Built = false;

private:
    std::vector<std::shared_ptr<Batch>> BuildBatchesFromRenderObjects(
        const std::vector<std::shared_ptr<RenderObject>>& ros);

    std::shared_ptr<Batch> FindBatchForRenderObject(const std::shared_ptr<RenderObject>& ro) const;
};