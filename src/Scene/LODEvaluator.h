#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

class RenderObject;
class Camera;

/**
 * A simple LOD evaluator that uses distance thresholds.
 * If distance > m_Distances[i], LOD increments.
 */
class LODEvaluator {
public:
    // Evaluate LOD for each object based on camera position and return new LOD indices.
    std::unordered_map<RenderObject*, size_t> EvaluateLODs(
        const std::vector<std::shared_ptr<RenderObject>>& objects,
        const std::shared_ptr<Camera>& camera);

private:
    // The old approach: fixed distance thresholds
    float m_Distances[4] = { 50.0f, 100.0f, 200.0f, 400.0f };
    //float m_Distances[4] = { 2.0f, 4.0f, 6.0f, 8.0f }; test lods
};