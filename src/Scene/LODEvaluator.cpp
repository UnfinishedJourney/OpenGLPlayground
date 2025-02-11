#include "LODEvaluator.h"
#include "Renderer/RenderObject.h"
#include "Scene/Camera.h"
#include <glm/glm.hpp>
#include <algorithm> // for std::min, std::max
#include <cmath>

std::unordered_map<BaseRenderObject*, size_t> LODEvaluator::EvaluateLODs(
    const std::vector<std::shared_ptr<BaseRenderObject>>& objects,
    const std::shared_ptr<Scene::Camera>& camera)
{
    std::unordered_map<BaseRenderObject*, size_t> lodMap;

    if (!camera) {
        // No camera => everything at LOD0
        for (auto& ro : objects) {
            lodMap[ro.get()] = 0;
        }
        return lodMap;
    }

    // Compute dynamic thresholds based on the camera's far plane.
    float farPlane = camera->GetFarPlane();
    float thresholds[4];
    for (int i = 0; i < 4; ++i) {
        thresholds[i] = m_Ratios[i] * farPlane;
    }

    glm::vec3 camPos = camera->GetPosition();

    for (auto& ro : objects) {
        glm::vec3 worldCenter = ro->GetCenter();
        float radius = ro->GetBoundingSphereRadius();
        float distance = glm::distance(camPos, worldCenter) - radius;
        if (distance < 0.0f) distance = 0.0f;

        size_t lodLevel = 0;
        // Now use our computed thresholds
        for (int i = 0; i < 4; ++i) {
            if (distance > thresholds[i]) {
                lodLevel++;
            }
            else {
                break;
            }
        }

        // Clamp to max available LOD for the object's mesh.
        size_t maxLOD = std::max<size_t>(1, ro->GetMesh()->GetLODCount()) - 1;
        lodLevel = std::min(lodLevel, maxLOD);

        lodMap[ro.get()] = lodLevel;
    }

    return lodMap;
}