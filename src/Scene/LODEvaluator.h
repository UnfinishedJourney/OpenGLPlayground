#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "Renderer/RenderObject.h"
#include "Scene/Camera.h"

class LODEvaluator {
public:
    // Evaluate LOD for each object based on camera position and return new LODs
    std::unordered_map<RenderObject*, size_t> EvaluateLODs(
        const std::vector<std::shared_ptr<RenderObject>>& objects,
        const std::shared_ptr<Camera>& camera);

private:
    // Example thresholds, adjust as needed
    float m_Distances[4] = { 50.0f, 100.0f, 200.0f, 400.0f };
};