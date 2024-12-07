#include "LODEvaluator.h"
#include <glm/glm.hpp>

std::unordered_map<RenderObject*, size_t> LODEvaluator::EvaluateLODs(
    const std::vector<std::shared_ptr<RenderObject>>& objects,
    const std::shared_ptr<Camera>& camera)
{
    std::unordered_map<RenderObject*, size_t> lodMap;
    glm::vec3 cameraPosition = camera->GetPosition();

    for (const auto& ro : objects) {
        glm::vec3 worldCenter = ro->GetWorldCenter();
        float radius = ro->GetBoundingSphereRadius();
        float distance = glm::distance(cameraPosition, worldCenter) - radius;

        // Determine LOD level based on distance
        size_t lodLevel = 0;
        for (float threshold : m_Distances) {
            if (distance > threshold) {
                lodLevel++;
            }
            else {
                break;
            }
        }

        size_t maxLOD = ro->GetMesh()->GetLODCount() - 1;
        if (lodLevel > maxLOD) {
            lodLevel = maxLOD;
        }

        lodMap[ro.get()] = lodLevel;
    }

    return lodMap;
}