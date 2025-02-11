#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

class BaseRenderObject;
namespace Scene
{
    class Camera;
}

/**
 * A simple LOD evaluator that uses distance thresholds.
 * If distance > m_Distances[i], LOD increments.
 */
class LODEvaluator {
public:
    // Evaluate LOD for each object based on camera position and return new LOD indices.
    std::unordered_map<BaseRenderObject*, size_t> EvaluateLODs(
        const std::vector<std::shared_ptr<BaseRenderObject>>& objects,
        const std::shared_ptr<Scene::Camera>& camera);

private:
    // Instead of fixed absolute distances, we define ratios.
    // For instance, if farPlane is 150 and the ratios are 0.25, 0.5, 0.75, 1.0,
    // the thresholds will be 37.5, 75, 112.5, and 150.
    float m_Ratios[4] = { 0.25f, 0.50f, 0.75f, 1.0f };
};