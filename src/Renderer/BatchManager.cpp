#include "BatchManager.h"
#include "Batch.h"
#include "Renderer/RenderObject.h"
#include "Scene/LODEvaluator.h"
#include "Scene/Camera.h"
#include "Utilities/Logger.h"
#include <algorithm>

void BatchManager::AddRenderObject(const std::shared_ptr<BaseRenderObject>& ro) {
    renderObjects_.push_back(ro);
    built_ = false;
}

void BatchManager::Clear() {
    renderObjects_.clear();
    batches_.clear();
    objToBatch_.clear();
    built_ = false;
}

const std::vector<std::shared_ptr<renderer::Batch>>& BatchManager::GetBatches() const {
    return batches_;
}

void BatchManager::BuildBatches() {
    if (built_) {
        return;
    }
    batches_.clear();
    objToBatch_.clear();

    if (!renderObjects_.empty()) {
        auto builtBatches = BuildBatchesFromObjects(renderObjects_);
        batches_.insert(batches_.end(), builtBatches.begin(), builtBatches.end());
    }
    built_ = true;
}

std::vector<std::shared_ptr<renderer::Batch>> BatchManager::BuildBatchesFromObjects(
    const std::vector<std::shared_ptr<BaseRenderObject>>& objs)
{
    // Group objects by shader name and material ID.
    using RenderObjVec = std::vector<std::shared_ptr<BaseRenderObject>>;
    using MaterialMap = std::unordered_map<int, RenderObjVec>;
    std::unordered_map<std::string, MaterialMap> grouping;

    for (auto& ro : objs) {
        grouping[ro->GetShaderName()][ro->GetMaterialID()].push_back(ro);
    }

    std::vector<std::shared_ptr<renderer::Batch>> result;
    for (auto& [shaderName, matMap] : grouping) {
        for (auto& [matID, objVec] : matMap) {
            auto batch = std::make_shared<renderer::Batch>(shaderName, matID);
            for (auto& ro : objVec) {
                batch->AddRenderObject(ro);
                objToBatch_[ro.get()] = batch;
            }
            batch->BuildBatches();
            result.push_back(batch);
        }
    }
    return result;
}

std::shared_ptr<renderer::Batch> BatchManager::FindBatchForObject(const std::shared_ptr<BaseRenderObject>& ro) const {
    auto it = objToBatch_.find(ro.get());
    return (it != objToBatch_.end()) ? it->second : nullptr;
}

void BatchManager::UpdateLOD(const std::shared_ptr<BaseRenderObject>& ro, size_t newLOD) {
    auto batch = FindBatchForObject(ro);
    if (!batch)
        return;
    auto& objects = batch->GetRenderObjects();
    auto it = std::find(objects.begin(), objects.end(), ro);
    if (it == objects.end())
        return;
    size_t idx = std::distance(objects.begin(), it);
    batch->UpdateLOD(idx, newLOD);
}

void BatchManager::UpdateLODs(std::shared_ptr<Scene::Camera>& camera, LODEvaluator& lodEvaluator) {
    if (!built_ || !camera)
        return;
    auto lodMap = lodEvaluator.EvaluateLODs(renderObjects_, camera);
    for (auto& ro : renderObjects_) {
        auto it = lodMap.find(ro.get());
        if (it != lodMap.end()) {
            size_t newLOD = it->second;
            if (newLOD != ro->GetCurrentLOD()) {
                UpdateLOD(ro, newLOD);
            }
        }
    }
}

void BatchManager::SetLOD(size_t forcedLOD) {
    if (!built_)
        return;
    for (auto& batch : batches_) {
        auto& ros = batch->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); i++) {
            auto ro = ros[i];
            if (ro->SetLOD(forcedLOD)) {
                batch->UpdateLOD(i, forcedLOD);
            }
        }
    }
}

void BatchManager::CullObject(const std::shared_ptr<BaseRenderObject>& ro) {
    auto batch = FindBatchForObject(ro);
    if (!batch)
        return;
    auto& ros = batch->GetRenderObjects();
    auto it = std::find(ros.begin(), ros.end(), ro);
    if (it == ros.end())
        return;
    size_t idx = std::distance(ros.begin(), it);
    batch->CullObject(idx);
}