#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Renderer/Batch.h"
#include "Scene/Camera.h"

class LODEvaluator;
class RenderObject;  // forward declaration

/**
 * @brief BatchManager accumulates RenderObjects and groups them by (Shader, Material, MeshLayout, Transform).
 *        Then it builds a list of Batches. Each Batch can issue a multi-draw.
 */
class BatchManager {
public:
    void AddRenderObject(const std::shared_ptr<RenderObject>& ro);
    void BuildBatches();
    void Clear();

    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;

    // LOD updates
    void UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD);
    void UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator);

    // Optional: force setting LOD for all objects (debug/dev tool)
    void SetLOD(size_t newLOD);

    // Culling
    void CullObject(const std::shared_ptr<RenderObject>& ro);

private:
    // Stores all RenderObjects that have been added.
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
    // All built batches.
    std::vector<std::shared_ptr<Batch>> m_AllBatches;
    // Lookup mapping: RenderObject raw pointer -> Batch that contains it.
    std::unordered_map<RenderObject*, std::shared_ptr<Batch>> m_ROBatchMap;
    bool m_Built = false;

private:
    // Build batches from a vector of RenderObjects, grouping by (ShaderName, MaterialName, Transform)
    std::vector<std::shared_ptr<Batch>> BuildBatchesFromRenderObjects(
        const std::vector<std::shared_ptr<RenderObject>>& ros);

    // Quickly find the Batch that contains the given RenderObject.
    std::shared_ptr<Batch> FindBatchForRenderObject(const std::shared_ptr<RenderObject>& ro) const;
};