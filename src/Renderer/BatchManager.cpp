#include "Renderer/BatchManager.h"
#include "Utilities/Logger.h"

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

    // Clear existing batches
    m_StaticBatches.clear();
    m_DynamicBatches.clear();

    if (m_RenderObjects.empty()) {
        return;
    }

    // For now, treat all render objects as static
    m_StaticBatches = BuildBatchesFromRenderObjects(m_RenderObjects);

    // Combine all batches into m_AllBatches
    m_AllBatches.clear();
    m_AllBatches.reserve(m_StaticBatches.size() + m_DynamicBatches.size());
    m_AllBatches.insert(m_AllBatches.end(), m_StaticBatches.begin(), m_StaticBatches.end());
    m_AllBatches.insert(m_AllBatches.end(), m_DynamicBatches.begin(), m_DynamicBatches.end());

    b_wasBuilt = true;
}

std::vector<std::shared_ptr<Batch>> BatchManager::BuildBatchesFromRenderObjects(
    const std::vector<std::shared_ptr<RenderObject>>& renderObjects) {
    std::vector<std::shared_ptr<Batch>> batches;

    // Group RenderObjects by shader, material, and MeshLayout
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<MeshLayout, std::vector<std::shared_ptr<RenderObject>>>>> batchMap;

    for (const auto& ro : renderObjects) {
        const auto& shaderName = ro->GetShaderName();
        const auto& materialName = ro->GetMaterialName();
        const auto& meshLayout = ro->GetMeshLayout();

        batchMap[shaderName][materialName][meshLayout].push_back(ro);
    }

    // Create batches
    for (const auto& shaderPair : batchMap) {
        const auto& shaderName = shaderPair.first;
        for (const auto& materialPair : shaderPair.second) {
            const auto& materialName = materialPair.first;
            for (const auto& layoutPair : materialPair.second) {
                const auto& meshLayout = layoutPair.first;
                const auto& ros = layoutPair.second;

                auto batch = std::make_shared<Batch>(shaderName, materialName, meshLayout);

                for (const auto& ro : ros) {
                    batch->AddRenderObject(ro);
                }

                batch->Update(); // Build or update the batch
                batches.push_back(batch);
            }
        }
    }

    return batches;
}

void BatchManager::UpdateLOD(const std::shared_ptr<RenderObject>& renderObject, size_t newLOD) {
    // Find the batch containing this render object
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

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const {
    return m_AllBatches;
}