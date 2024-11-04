#include "Renderer/BatchManager.h"

void BatchManager::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_RenderObjects.push_back(renderObject);
}

void BatchManager::BuildBatches()
{
    m_Batches.clear();

    // Group RenderObjects by shader, material, and MeshLayout
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<MeshLayout, std::vector<std::shared_ptr<RenderObject>>>>> batchMap;

    for (const auto& ro : m_RenderObjects) {
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
                const auto& renderObjects = layoutPair.second;

                auto batch = std::make_unique<Batch>(shaderName, materialName, meshLayout);

                for (const auto& ro : renderObjects) {
                    batch->AddRenderObject(ro);
                }

                batch->BuildBatch();
                m_Batches.push_back(std::move(batch));
            }
        }
    }
}

const std::vector<std::unique_ptr<Batch>>& BatchManager::GetBatches() const
{
    return m_Batches;
}