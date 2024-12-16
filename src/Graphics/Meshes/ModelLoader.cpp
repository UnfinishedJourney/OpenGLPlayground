#include "ModelLoader.h"
#include "Resources/ModelManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include "Renderer/RenderObject.h"
#include "Scene/SceneNode.h"
#include "Utilities/Logger.h"

std::shared_ptr<SceneNode> ModelLoader::LoadModelIntoSceneGraph(
    const std::string& filePath,
    const std::string& defaultShaderName,
    const std::string& defaultMaterialName,
    const MeshLayout& layout)
{
    // For example, use your existing ModelManager
    auto& modelManager = ModelManager::GetInstance();
    auto model = modelManager.GetModel(filePath); // Maybe your manager keys by 'pig' or by path

    if (!model) {
        Logger::GetLogger()->error("Failed to load model '{}'", filePath);
        return nullptr;
    }

    // Create a root node for this model
    auto rootNode = std::make_shared<SceneNode>();

    // For each mesh in the model, create a child node
    const auto& meshInfos = model->GetMeshesInfo();
    for (const auto& meshInfo : meshInfos) {
        auto transform = std::make_shared<Transform>();
        // Optionally set a local transform here if needed
        // transform->SetPosition(...);

        auto ro = std::make_shared<RenderObject>(
            meshInfo.mesh,
            layout,
            defaultMaterialName,
            defaultShaderName,
            transform
        );

        auto meshNode = std::make_shared<SceneNode>();
        meshNode->SetRenderObject(ro);

        // Add to root
        rootNode->AddChild(meshNode);
    }

    return rootNode;
}