#include "Renderer/BatchManager.h"
#include "Utilities/Logger.h"
#include "Scene/LODEvaluator.h"
#include <algorithm>
#include <unordered_map>

void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& ro) {
    m_RenderObjects.push_back(ro);
    b_wasBuilt = false;
}

void BatchManager::Clear() {
    m_RenderObjects.clear();
    m_AllBatches.clear();
    b_wasBuilt = false;
}

void BatchManager::BuildBatches() {
    if (b_wasBuilt) return;

    m_AllBatches.clear();

    if (!m_RenderObjects.empty()) {
        auto newBatches = BuildBatchesFromRenderObjects(m_RenderObjects);
        m_AllBatches.insert(m_AllBatches.end(), newBatches.begin(), newBatches.end());
    }

    b_wasBuilt = true;
}

std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& ros)
{
    std::vector<std::shared_ptr<Batch>> batches;
    /**
     * Group by (shaderName, materialName, meshLayout).
     */
    std::unordered_map<std::string,
        std::unordered_map<std::string,
        std::unordered_map<MeshLayout, std::vector<std::shared_ptr<RenderObject>>>
        >
    > batchMap;

    for (auto& ro : ros) {
        const auto& shader = ro->GetShaderName();
        const auto& mat = ro->GetMaterialName();
        const auto& layout = ro->GetMeshLayout();
        batchMap[shader][mat][layout].push_back(ro);
    }

    for (auto& shaderPair : batchMap) {
        for (auto& matPair : shaderPair.second) {
            for (auto& layoutPair : matPair.second) {
                auto batch = std::make_shared<Batch>(shaderPair.first, matPair.first, layoutPair.first);
                for (auto& ro : layoutPair.second) {
                    batch->AddRenderObject(ro);
                }
                batch->Update();
                batches.push_back(batch);
            }
        }
    }

    return batches;
}

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const {
    return m_AllBatches;
}

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD) {
    // Find which batch the RenderObject is in, then update command
    for (auto& batch : m_AllBatches) {
        const auto& batchROs = batch->GetRenderObjects();
        auto it = std::find(batchROs.begin(), batchROs.end(), ro);
        if (it != batchROs.end()) {
            size_t idx = std::distance(batchROs.begin(), it);
            batch->UpdateLOD(idx, newLOD);
            break;
        }
    }
}

void BatchManager::UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator) {
    if (m_AllBatches.empty()) return;

    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);
    for (auto& ro : m_RenderObjects) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            size_t oldLOD = ro->GetCurrentLOD();
            if (newLOD != oldLOD) {
                if (ro->SetLOD(newLOD)) {
                    UpdateLOD(ro, newLOD);
                }
            }
        }
    }
}

void BatchManager::SetLOD(size_t newLOD) {
    // Force a single LOD level on all ROs in the scene
    for (auto& batch : m_AllBatches) {
        auto& batchROs = batch->GetRenderObjects();
        for (size_t i = 0; i < batchROs.size(); i++) {
            auto ro = batchROs[i];
            if (ro->SetLOD(newLOD)) {
                batch->UpdateLOD(i, newLOD);
            }
        }
    }
}