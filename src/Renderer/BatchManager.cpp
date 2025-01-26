#include "BatchManager.h"
#include "Renderer/Batch.h"
#include "Scene/LODEvaluator.h"
#include "Renderer/RenderObject.h"
#include "Utilities/Logger.h"
#include <algorithm>
#include <unordered_map>

// Add a RenderObject and mark the batches as needing rebuild.
void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& ro)
{
    m_RenderObjects.push_back(ro);
    m_Built = false;
}

// Clear render objects, batches, and the lookup map.
void BatchManager::Clear()
{
    m_RenderObjects.clear();
    m_AllBatches.clear();
    m_ROBatchMap.clear();
    m_Built = false;
}

// Build batches from RenderObjects.
void BatchManager::BuildBatches()
{
    if (m_Built) {
        return;
    }
    m_AllBatches.clear();
    m_ROBatchMap.clear();

    if (!m_RenderObjects.empty()) {
        auto newBatches = BuildBatchesFromRenderObjects(m_RenderObjects);
        m_AllBatches.insert(m_AllBatches.end(), newBatches.begin(), newBatches.end());
    }
    m_Built = true;
}

// Group render objects by (ShaderName, MaterialName, Transform) and build batches.
// Adjust the grouping scheme if you need to include MeshLayout or other parameters.
std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& ros)
{
    std::vector<std::shared_ptr<Batch>> batches;

    // We'll group as:
    //   ShaderName -> MaterialName -> Transform -> vector<RenderObject>
    using TransformMap = std::unordered_map<Transform, std::vector<std::shared_ptr<RenderObject>>>;
    using MaterialMap = std::unordered_map<int, TransformMap>;
    std::unordered_map<std::string, MaterialMap> bigMap;

    for (auto& ro : ros) {
        // Note: This assumes that ro->GetTransform() returns a pointer/reference to a Transform
        // and that Transform has valid operator== and std::hash specializations.
        bigMap[ro->GetShaderName()]
            [ro->GetMaterialID()]
            [*(ro->GetTransform())]
            .push_back(ro);
    }

    // For every group, create a Batch.
    for (auto& [shaderName, matMap] : bigMap) {
        for (auto& [matID, transformMap] : matMap) {
            for (auto& [transform, roVec] : transformMap) {
                // Create a new batch.
                // If your Batch constructor accepts a Transform (or MeshLayout) adjust accordingly.
                auto batch = std::make_shared<Batch>(shaderName, matID, transform);
                for (auto& ro : roVec) {
                    batch->AddRenderObject(ro);
                    // Populate the lookup: key is the raw pointer.
                    m_ROBatchMap[ro.get()] = batch;
                }
                batch->BuildBatches();
                batches.push_back(batch);
            }
        }
    }
    return batches;
}

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const
{
    return m_AllBatches;
}

// Use the lookup map to quickly find the Batch for the given RenderObject.
std::shared_ptr<Batch> BatchManager::FindBatchForRenderObject(const std::shared_ptr<RenderObject>& ro) const
{
    auto it = m_ROBatchMap.find(ro.get());
    if (it != m_ROBatchMap.end()) {
        return it->second;
    }
    return nullptr;
}

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD)
{
    auto batch = FindBatchForRenderObject(ro);
    if (!batch) return;

    // Find the render object’s index within the batch.
    auto& ros = batch->GetRenderObjects();
    auto it = std::find(ros.begin(), ros.end(), ro);
    if (it == ros.end()) return;

    size_t idx = std::distance(ros.begin(), it);
    batch->UpdateLOD(idx, newLOD);
}

void BatchManager::UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator)
{
    if (m_AllBatches.empty() || !camera) {
        return;
    }
    // Use the LODEvaluator to determine each RenderObject’s new LOD.
    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);

    for (auto& ro : m_RenderObjects) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            if (newLOD != ro->GetCurrentLOD()) {
                // Update the corresponding batch.
                UpdateLOD(ro, newLOD);
            }
        }
    }
}

void BatchManager::SetLOD(size_t newLOD)
{
    // Force the same LOD for every RenderObject (debug feature).
    for (auto& batch : m_AllBatches) {
        auto& ros = batch->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); i++) {
            if (ros[i]->SetLOD(newLOD)) {
                batch->UpdateLOD(i, newLOD);
            }
        }
    }
}

void BatchManager::CullObject(const std::shared_ptr<RenderObject>& ro)
{
    auto batch = FindBatchForRenderObject(ro);
    if (!batch) return;

    auto& ros = batch->GetRenderObjects();
    auto it = std::find(ros.begin(), ros.end(), ro);
    if (it == ros.end()) return;

    size_t idx = std::distance(ros.begin(), it);
    batch->CullObject(idx);
}