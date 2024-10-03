#pragma once

#include <memory>
#include <string>
#include <assimp/scene.h>
#include "Graphics/Meshes/Model.h"
#include "Resources/ResourceManager.h"

class ModelLoader {
public:
    ModelLoader(std::shared_ptr<ResourceManager> resourceManager);

    std::shared_ptr<Model> LoadModel(const std::string& path, bool centerModel = false);

private:
    void ProcessNode(const aiScene* scene, const aiNode* node, std::shared_ptr<Model> model);
    std::shared_ptr<Mesh> ProcessMesh(const aiScene* scene, const aiMesh* aiMesh);
    std::shared_ptr<Material> LoadMaterial(const aiScene* scene, aiMaterial* mat);

    std::shared_ptr<ResourceManager> m_ResourceManager;
    std::string m_Directory;
};