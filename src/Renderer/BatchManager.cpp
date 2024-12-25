#include "Renderer/BatchManager.h"
#include "Utilities/Logger.h"
#include "Scene/LODEvaluator.h"
#include <algorithm>
#include <unordered_map>
#include "Utilities/ProfilerMacros.h" 

void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& ro) {
    PROFILE_FUNCTION(Green); // Profile the AddRenderObject function

    m_RenderObjects.push_back(ro);
    b_wasBuilt = false;
}

void BatchManager::Clear() {
    PROFILE_FUNCTION(Green); // Profile the Clear function

    m_RenderObjects.clear();
    m_AllBatches.clear();
    b_wasBuilt = false;
}

void BatchManager::BuildBatches() {
    PROFILE_FUNCTION(Cyan); // Profile the BuildBatches function

    if (b_wasBuilt) return;

    {
        PROFILE_BLOCK("Clear Existing Batches", Yellow);
        m_AllBatches.clear();
        // Profiling block ends here
    }

    if (!m_RenderObjects.empty()) {
        {
            PROFILE_BLOCK("Build New Batches from RenderObjects", Yellow);
            auto newBatches = BuildBatchesFromRenderObjects(m_RenderObjects);
            m_AllBatches.insert(m_AllBatches.end(), newBatches.begin(), newBatches.end());
            // Profiling block ends here
        }
    }

    b_wasBuilt = true;
}

std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& ros)
{
    PROFILE_FUNCTION(Yellow); // Profile the BuildBatchesFromRenderObjects function

    std::vector<std::shared_ptr<Batch>> batches;

    /**
     * Group by (shaderName, materialName, meshLayout).
     */
    std::unordered_map<std::string,
        std::unordered_map<std::string,
        std::unordered_map<MeshLayout, std::vector<std::shared_ptr<RenderObject>>>
        >
    > batchMap;

    {
        PROFILE_BLOCK("Group RenderObjects by Shader, Material, and MeshLayout", Yellow);
        for (auto& ro : ros) {
            const auto& shader = ro->GetShaderName();
            const auto& mat = ro->GetMaterialName();
            const auto& layout = ro->GetMeshLayout();
            batchMap[shader][mat][layout].push_back(ro);
        }
        // Profiling block ends here
    }

    {
        PROFILE_BLOCK("Create Batches from Grouped RenderObjects", Yellow);
        for (auto& shaderPair : batchMap) {
            for (auto& matPair : shaderPair.second) {
                for (auto& layoutPair : matPair.second) {
                    auto batch = std::make_shared<Batch>(shaderPair.first, matPair.first, layoutPair.first);
                    for (auto& ro : layoutPair.second) {
                        batch->AddRenderObject(ro);
                    }
                    batch->BuildBatches(); // Assuming BuildBatches internally calls Update()
                    batches.push_back(batch);
                }
            }
        }
        // Profiling block ends here
    }

    return batches;
}

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const {
    PROFILE_FUNCTION(Yellow); // Profile the GetBatches function
    return m_AllBatches;
}

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& ro, size_t newLOD) {
    PROFILE_FUNCTION(Yellow); // Profile the UpdateLOD function

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
    PROFILE_FUNCTION(Cyan); // Profile the UpdateLODs function

    if (m_AllBatches.empty()) return;

    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);

    {
        PROFILE_BLOCK("Update LODs for RenderObjects", Yellow);
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
        // Profiling block ends here
    }
}

void BatchManager::SetLOD(size_t newLOD) {
    PROFILE_FUNCTION(Yellow); // Profile the SetLOD function

    // Force a single LOD level on all ROs in the scene
    {
        PROFILE_BLOCK("Set Single LOD for All RenderObjects", Yellow);
        for (auto& batch : m_AllBatches) {
            auto& batchROs = batch->GetRenderObjects();
            for (size_t i = 0; i < batchROs.size(); i++) {
                auto ro = batchROs[i];
                if (ro->SetLOD(newLOD)) {
                    batch->UpdateLOD(i, newLOD);
                }
            }
        }
        // Profiling block ends here
    }
}