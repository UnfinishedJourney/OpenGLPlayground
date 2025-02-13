#include "BatchManager.h"
#include "Batch.h"
#include "Renderer/RenderObject.h"
#include "Scene/LODEvaluator.h"
#include "Scene/Camera.h"
#include "Utilities/Logger.h"

#include <algorithm>

void BatchManager::AddRenderObject(const std::shared_ptr<BaseRenderObject>& ro)
{
    m_RenderObjects.push_back(ro);
    m_Built = false;
}

void BatchManager::Clear()
{
    m_RenderObjects.clear();
    m_Batches.clear();
    m_ObjToBatch.clear();
    m_Built = false;
}

const std::vector<std::shared_ptr<renderer::Batch>>& BatchManager::GetBatches() const
{
    return m_Batches;
}

void BatchManager::BuildBatches()
{
    if (m_Built) {
        return;
    }
    m_Batches.clear();
    m_ObjToBatch.clear();

    if (!m_RenderObjects.empty()) {
        auto built = BuildBatchesFromObjects(m_RenderObjects);
        m_Batches.insert(m_Batches.end(), built.begin(), built.end());
    }
    m_Built = true;
}

// Groups objects by (shaderName, materialID) => new Batch for each group
std::vector<std::shared_ptr<renderer::Batch>> BatchManager::BuildBatchesFromObjects(
    const std::vector<std::shared_ptr<BaseRenderObject>>& objs)
{
    // (shaderName) -> (materialID) -> vector of objects
    using RenderObjVec = std::vector<std::shared_ptr<BaseRenderObject>>;
    using MaterialMap = std::unordered_map<int, RenderObjVec>;
    std::unordered_map<std::string, MaterialMap> grouping;

    for (auto& ro : objs) {
        grouping[ro->GetShaderName()][ro->GetMaterialID()].push_back(ro);
    }

    std::vector<std::shared_ptr<renderer::Batch>> result;
    result.reserve(grouping.size());

    // For each group => create a Batch
    for (auto& [shader, matMap] : grouping) {
        for (auto& [matID, objVec] : matMap) {
            auto batch = std::make_shared<renderer::Batch>(shader, matID);
            for (auto& ro : objVec) {
                batch->AddRenderObject(ro);
                m_ObjToBatch[ro.get()] = batch;
            }
            // Build GPU data
            batch->BuildBatches();
            result.push_back(batch);
        }
    }
    return result;
}

std::shared_ptr<renderer::Batch> BatchManager::FindBatchForObject(const std::shared_ptr<BaseRenderObject>& ro) const
{
    auto it = m_ObjToBatch.find(ro.get());
    if (it != m_ObjToBatch.end()) {
        return it->second;
    }
    return nullptr;
}

// Called when an external LOD evaluator says "object X => newLOD".
void BatchManager::UpdateLOD(const std::shared_ptr<BaseRenderObject>& ro, size_t newLOD)
{
    auto batch = FindBatchForObject(ro);
    if (!batch) return;

    // find the index of this object in the batch
    auto& objects = batch->GetRenderObjects();
    auto it = std::find(objects.begin(), objects.end(), ro);
    if (it == objects.end()) return; // not found

    size_t idx = std::distance(objects.begin(), it);
    batch->UpdateLOD(idx, newLOD);
}

// Evaluate LOD for each object using the LODEvaluator
void BatchManager::UpdateLODs(std::shared_ptr<Scene::Camera>& camera, LODEvaluator& lodEvaluator)
{
    if (!m_Built || !camera) {
        return;
    }
    auto lodMap = lodEvaluator.EvaluateLODs(m_RenderObjects, camera);

    for (auto& ro : m_RenderObjects) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            if (newLOD != ro->GetCurrentLOD()) {
                UpdateLOD(ro, newLOD);
            }
        }
    }
}

// Force all objects to the same LOD
void BatchManager::SetLOD(size_t forcedLOD)
{
    if (!m_Built) {
        return;
    }
    for (auto& batch : m_Batches) {
        auto& ros = batch->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); i++) {
            auto ro = ros[i];
            if (ro->SetLOD(forcedLOD)) {
                batch->UpdateLOD(i, forcedLOD);
            }
        }
    }
}

// Mark an object as culled => set draw count=0
void BatchManager::CullObject(const std::shared_ptr<BaseRenderObject>& ro)
{
    auto batch = FindBatchForObject(ro);
    if (!batch) return;

    auto& ros = batch->GetRenderObjects();
    auto it = std::find(ros.begin(), ros.end(), ro);
    if (it == ros.end()) return;

    size_t idx = std::distance(ros.begin(), it);
    batch->CullObject(idx);
}