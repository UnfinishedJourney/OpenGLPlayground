#pragma once

#include <vector>
#include <memory>
#include "Renderer/Batch.h"
#include "Renderer/RenderObject.h"
#include "Scene/Camera.h"

class LODEvaluator; // optional forward declare

class BatchManager {
public:
    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void BuildBatches();
    void Clear();
    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;

    // LOD usage
    void UpdateLOD(const std::shared_ptr<RenderObject>& renderObject, size_t newLOD);
    void UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator);
    void SetLOD(size_t newLOD);

private:
    bool b_wasBuilt = false;
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
    std::vector<std::shared_ptr<Batch>> m_AllBatches;

    std::vector<std::shared_ptr<Batch>> BuildBatchesFromRenderObjects(const std::vector<std::shared_ptr<RenderObject>>& ros);
};