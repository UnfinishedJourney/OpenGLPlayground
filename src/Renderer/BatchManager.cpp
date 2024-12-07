#include "Renderer/BatchManager.h"
#include "Utilities/Logger.h"
#include "Scene/LODEvaluator.h"
#include <algorithm>
#include <unordered_map>

void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject) {
    m_RenderObjects.push_back(renderObject);
    b_wasBuilt = false;
}

void BatchManager::Clear() {
    m_RenderObjects.clear();
    m_StaticBatches.clear();
    m_DynamicBatches.clear();
    m_AllBatches.clear();
    b_wasBuilt = false;
}

void BatchManager::BuildBatches() {
    if (b_wasBuilt) return;

    m_StaticBatches.clear();
    m_DynamicBatches.clear();

    if (!m_RenderObjects.empty()) {
        m_StaticBatches = BuildBatchesFromRenderObjects(m_RenderObjects);
    }

    m_AllBatches.clear();
    m_AllBatches.reserve(m_StaticBatches.size() + m_DynamicBatches.size());
    m_AllBatches.insert(m_AllBatches.end(), m_StaticBatches.begin(), m_StaticBatches.end());
    m_AllBatches.insert(m_AllBatches.end(), m_DynamicBatches.begin(), m_DynamicBatches.end());

    b_wasBuilt = true;
}

std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& renderObjects) {
    std::vector<std::shared_ptr<Batch>> batches;
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<MeshLayout, std::vector<std::shared_ptr<RenderObject>>>>> batchMap;

    for (const auto& ro : renderObjects) {
        const auto& shaderName = ro->GetShaderName();
        const auto& materialName = ro->GetMaterialName();
        const auto& meshLayout = ro->GetMeshLayout();
        batchMap[shaderName][materialName][meshLayout].push_back(ro);
    }

    for (const auto& shaderPair : batchMap) {
        for (const auto& materialPair : shaderPair.second) {
            for (const auto& layoutPair : materialPair.second) {
                auto batch = std::make_shared<Batch>(shaderPair.first, materialPair.first, layoutPair.first);
                for (const auto& ro : layoutPair.second) {
                    batch->AddRenderObject(ro);
                }
                batch->Update();
                batches.push_back(batch);
            }
        }
    }

    return batches;
}

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& renderObject, size_t newLOD) {
    for (const auto& batch : m_AllBatches) {
        const auto& ros = batch->GetRenderObjects();
        auto it = std::find(ros.begin(), ros.end(), renderObject);
        if (it != ros.end()) {
            size_t index = std::distance(ros.begin(), it);
            batch->UpdateLOD(index, newLOD);
            break;
        }
    }
}

void BatchManager::UpdateLODs(std::shared_ptr<Camera>& camera, LODEvaluator& lodEvaluator) {
    if (m_AllBatches.empty()) return;

    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);

    // Update only if LOD changed in RenderObject and reflect in batch
    for (auto& ro : m_RenderObjects) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            size_t oldLOD = ro->GetCurrentLOD();
            if (newLOD != oldLOD) {
                if (ro->SetLOD(newLOD)) {
                    // LOD actually changed, update batch commands
                    UpdateLOD(ro, newLOD);
                }
            }
        }
    }
}

void BatchManager::SetLOD(size_t newLOD) {
    for (const auto& batch : m_AllBatches) {
        const auto& ros = batch->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); i++) {
            auto& ro = ros[i];
            if (ro->SetLOD(newLOD)) {
                batch->UpdateLOD(i, newLOD);
            }
        }
    }
}

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const {
    return m_AllBatches;
}