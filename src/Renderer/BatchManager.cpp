#include "BatchManager.h"
#include "Scene/LODEvaluator.h"
#include "Utilities/Logger.h"
#include <algorithm>
#include <unordered_map>

void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& ro)
{
    m_RenderObjects.push_back(ro);
    m_Built = false;
}

void BatchManager::Clear()
{
    m_RenderObjects.clear();
    m_AllBatches.clear();
    m_Built = false;
}

void BatchManager::BuildBatches()
{
    if (m_Built) {
        return;
    }
    m_AllBatches.clear();

    if (!m_RenderObjects.empty()) {
        auto newBatches = BuildBatchesFromRenderObjects(m_RenderObjects);
        m_AllBatches.insert(m_AllBatches.end(), newBatches.begin(), newBatches.end());
    }
    m_Built = true;
}

std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& ros)
{
    std::vector<std::shared_ptr<Batch>> batches;

    // We'll group by (ShaderName, MaterialName, MeshLayout)
    // That triple is the key for one batch
    using TransformMap = std::unordered_map<Transform, std::vector<std::shared_ptr<RenderObject>>>;
    using MaterialMap = std::unordered_map<std::string, TransformMap>;
    std::unordered_map<std::string, MaterialMap> bigMap;

    for (auto& ro : ros) {
        bigMap[ro->GetShaderName()]
            [ro->GetMaterialName()]
            [*(ro->GetTransform())]
            .push_back(ro);
    }

    // Then build each batch
    for (auto& [shaderName, matMap] : bigMap) {
        for (auto& [matName, transformMap] : matMap) {
            for (auto& [transform, roVec] : transformMap) {
                auto batch = std::make_shared<Batch>(shaderName, matName, transform);
                for (auto& ro : roVec) {
                    batch->AddRenderObject(ro);
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

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD)
{
    auto batch = FindBatchForRenderObject(ro);
    if (!batch) return;

    // Find ro's index in that batch
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
    // Use the LODEvaluator to determine each RO's new LOD
    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);

    for (auto& ro : m_RenderObjects) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            if (newLOD != ro->GetCurrentLOD()) {
                // Actually update the batch
                UpdateLOD(ro, newLOD);
            }
        }
    }
}

void BatchManager::SetLOD(size_t newLOD)
{
    // Force the same LOD for everything (debug feature)
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

std::shared_ptr<Batch> BatchManager::FindBatchForRenderObject(const std::shared_ptr<RenderObject>& ro) const
{
    // Search each batch to find the RO
    // If there's a lot of ROs, consider caching an "RO -> batch" map
    for (auto& batch : m_AllBatches) {
        auto& ros = batch->GetRenderObjects();
        if (std::find(ros.begin(), ros.end(), ro) != ros.end()) {
            return batch;
        }
    }
    return nullptr;
}