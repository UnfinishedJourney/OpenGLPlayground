#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

ModelLoader::ModelLoader(std::shared_ptr<ResourceManager> resourceManager)
    : m_ResourceManager(resourceManager)
{
}

std::shared_ptr<Model> ModelLoader::LoadModel(const std::string& path, bool centerModel)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Couldn't load model: " << path << "\n";
        return nullptr;
    }

    m_Directory = path.substr(0, path.find_last_of('/'));

    std::shared_ptr<Model> model = std::make_shared<Model>();

    ProcessNode(scene, scene->mRootNode, model);

    if (centerModel) {
        model->CenterModel();
    }

    return model;
}

void ModelLoader::ProcessNode(const aiScene* scene, const aiNode* node, std::shared_ptr<Model> model)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
        std::shared_ptr<Mesh> mesh = ProcessMesh(scene, aiMesh);
        model->AddMesh(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(scene, node->mChildren[i], model);
    }
}

std::shared_ptr<Mesh> ModelLoader::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::shared_ptr<Material> material;
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
        material = LoadMaterial(scene, aiMat);
    }

    mesh->SetMaterial(material);

    return mesh;
}

std::shared_ptr<Material> ModelLoader::LoadMaterial(const aiScene* scene, aiMaterial* aiMat)
{
    std::shared_ptr<Material> material = std::make_shared<Material>();

    std::shared_ptr<Shader> shader = m_ResourceManager->GetShader("defaultShader");
    material->SetShader(shader);

    auto LoadTextureOfType = [this, aiMat, material](aiTextureType aiType, TextureType type) {
        for (unsigned int i = 0; i < aiMat->GetTextureCount(aiType); i++) {
            aiString str;
            aiMat->GetTexture(aiType, i, &str);
            std::string texturePath = m_Directory + '/' + str.C_Str();
            std::shared_ptr<Texture2D> texture = m_ResourceManager->GetTexture(texturePath);
            if (!texture) {
                texture = std::make_shared<Texture2D>(texturePath);
                m_ResourceManager->AddTexture(texturePath, texture);
            }
            material->SetTexture(type, texture);
        }
        };

    LoadTextureOfType(aiTextureType_DIFFUSE, TextureType::Albedo);
    LoadTextureOfType(aiTextureType_NORMALS, TextureType::Normal);
    LoadTextureOfType(aiTextureType_METALNESS, TextureType::Metallic);
    LoadTextureOfType(aiTextureType_DIFFUSE_ROUGHNESS, TextureType::Roughness);
    LoadTextureOfType(aiTextureType_AMBIENT_OCCLUSION, TextureType::AmbientOcclusion);
    LoadTextureOfType(aiTextureType_EMISSION_COLOR, TextureType::Emissive);

    return material;
}