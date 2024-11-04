#pragma once

#include <vector>
#include <memory>
#include "Renderer/Batch.h"
#include "Renderer/RenderObject.h"

class BatchManager {
public:
    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void BuildBatches();
    const std::vector<std::unique_ptr<Batch>>& GetBatches() const;

private:
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
    std::vector<std::unique_ptr<Batch>> m_Batches;
};