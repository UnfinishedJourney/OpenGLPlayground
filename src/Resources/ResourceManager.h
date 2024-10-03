#pragma once

#include "Graphics/Meshes/Model.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Renderer/RenderObject.h"
#include "Resources/ShaderManager.h"
#include "Scene/Transform.h"

struct MeshKey {
    std::string name;
    MeshLayout layout;

    // Define equality operator for comparison
    bool operator==(const MeshKey& other) const {
        return name == other.name && layout == other.layout;
    }
};

namespace std {
    template <>
    struct hash<MeshKey> {
        std::size_t operator()(const MeshKey& key) const {
            std::size_t h1 = std::hash<std::string>()(key.name);
            std::size_t h2 = std::hash<MeshLayout>()(key.layout);
            return h1 ^ (h2 << 1); // Combine the hashes
        }
    };
}

class ResourceManager {
public:
    ResourceManager();
    std::shared_ptr<Texture2D> GetTexture(const std::string& textureName);
    bool DeleteTexture(const std::string& textureName);

    std::shared_ptr<Mesh> GetMesh(const std::string& meshName);
    bool DeleteMesh(const std::string& meshName);

    std::shared_ptr<Model> GetModel(const std::string& modelName);
    bool DeleteModel(const std::string& modelName);

    std::shared_ptr<Shader> GetShader(const std::string& shaderName);

    std::shared_ptr<ComputeShader> GetComputeShader(const std::string& shaderName);

    std::shared_ptr<MeshBuffer> GetMeshBuffer(const std::string& meshName, const MeshLayout& layout);
    bool DeleteMeshBuffer(const std::string& meshName, MeshLayout layout);
    void ReloadAllShaders()
    {
        m_ShaderManager->ReloadAllShaders();
    }

private:
    std::unique_ptr<ShaderManager> m_ShaderManager;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<MeshKey, std::shared_ptr<MeshBuffer>> m_MeshBuffers;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
    std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
};