#include "Renderer/BatchManager.h"
#include "Utilities/Logger.h"

//need more lods
struct LODThresholds {
    float distances[4]; // Adjust the number of LODs as needed
} g_LODThresholds = { { 50.0f, 100.0f, 200.0f, 400.0f } };



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

void BatchManager::UpdateLODs(std::shared_ptr<Camera>& camera)
{
    glm::vec3 cameraPosition = camera->GetPosition();

    for (const auto& batch : m_AllBatches) {
        const auto& renderObjects = batch->GetRenderObjects();
        for (size_t objectIndex = 0; objectIndex < renderObjects.size(); ++objectIndex) {
            const auto& ro = renderObjects[objectIndex];
            glm::vec3 worldCenter = ro->GetWorldCenter();
            float radius = ro->GetBoundingSphereRadius();

            // Compute distance from camera to mesh bounding sphere
            float distance = glm::distance(cameraPosition, worldCenter) - radius;

            // Determine LOD level based on distance
            size_t lodLevel = 0;
            for (size_t i = 0; i < std::size(g_LODThresholds.distances); ++i) {
                if (distance > g_LODThresholds.distances[i]) {
                    lodLevel = i + 1;
                }
                else {
                    break;
                }
            }

            // Clamp LOD level to available LODs
            size_t maxLOD = ro->GetMesh()->GetLODCount() - 1;
            lodLevel = std::min(lodLevel, maxLOD);

            // Update LOD in batch
            batch->UpdateLOD(objectIndex, lodLevel);
        }
    }
}

void BatchManager::SetLOD(size_t newLOD) {
    
    for (const auto& batch : m_AllBatches)
    {
        int i = 0;
        for (auto& ros : batch->GetRenderObjects())
        {
            batch->UpdateLOD(i++, newLOD);
        }
    }
}

const std::vector<std::shared_ptr<Batch>>& BatchManager::GetBatches() const {
    return m_AllBatches;
}