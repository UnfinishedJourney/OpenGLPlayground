#include "LODEvaluator.h"
#include "Renderer/RenderObject.h"
#include "Scene/Camera.h"
#include <glm/glm.hpp>
#include <algorithm> // for std::min, std::max
#include <cmath>

std::unordered_map<RenderObject*, size_t> LODEvaluator::EvaluateLODs(
    const std::vector<std::shared_ptr<RenderObject>>& objects,
    const std::shared_ptr<Camera>& camera)
{
    std::unordered_map<RenderObject*, size_t> lodMap;

    if (!camera) {
        // No camera => everything at LOD0
        for (auto& ro : objects) {
            lodMap[ro.get()] = 0;
        }
        return lodMap;
    }

    glm::vec3 camPos = camera->GetPosition();

    for (auto& ro : objects) {
        glm::vec3 worldCenter = ro->GetWorldCenter();
        float radius = ro->GetBoundingSphereRadius();
        float distance = glm::distance(camPos, worldCenter) - radius;
        if (distance < 0.0f) distance = 0.0f;

        size_t lodLevel = 0;
        for (float threshold : m_Distances) {
            if (distance > threshold) {
                lodLevel++;
            }
            else {
                break;
            }
        }

        // Clamp to max LOD
        size_t maxLOD = std::max<size_t>(1, ro->GetMesh()->GetLODCount()) - 1;
        lodLevel = std::min(lodLevel, maxLOD);

        lodMap[ro.get()] = lodLevel;
    }

    return lodMap;
}
