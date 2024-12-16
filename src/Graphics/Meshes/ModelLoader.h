#pragma once
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include "Scene/SceneNode.h"
#include "Graphics/Meshes/MeshLayout.h"

class ModelLoader {
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    /**
     * Loads a model from the given file path, creates a hierarchy of SceneNodes,
     * each with a RenderObject (and its mesh), and returns the root node.
     *
     * If you want to share a common shaderName/materialName, pass them in.
     * For a more advanced approach, you might parse the actual node transforms from Assimp
     * and recursively build the same hierarchy.
     */
    std::shared_ptr<SceneNode> LoadModelIntoSceneGraph(
        const std::string& filePath,
        const std::string& defaultShaderName,
        const std::string& defaultMaterialName,
        const MeshLayout& layout
    );

private:
    // Optionally store logic or references to load from your existing Model class
    // e.g. ModelManager, or direct Assimp usage, etc.
};