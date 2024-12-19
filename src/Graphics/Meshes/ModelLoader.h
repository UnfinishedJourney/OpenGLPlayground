#pragma once
#include <string>
#include <vector>
#include "Scene/SceneGraph.h"
#include "MeshLayout.h"
#include "Model.h"
#include <filesystem>

class ModelLoader {
public:
    bool LoadIntoSceneGraph(
        const Model& model,
        const MeshLayout& layout,
        SceneGraph& sceneGraph,
        std::vector<MeshInfo>& outMeshes,
        std::vector<std::string>& outMaterials);
};