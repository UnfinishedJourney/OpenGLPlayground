#pragma once

#include "Graphics/Meshes/Mesh.h"
//#include "Graphics/Meshes/Model.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/CubeMapTexture.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Scene/Transform.h"


struct MeshKey {
    std::string name;
    MeshLayout meshLayout;

    // Define equality operator for comparison
    bool operator==(const MeshKey& other) const {
        return name == other.name && meshLayout == other.meshLayout;
    }
};

namespace std {
    template <>
    struct hash<MeshKey> {
        std::size_t operator()(const MeshKey& key) const {
            std::size_t h1 = std::hash<std::string>()(key.name);
            std::size_t h2 = std::hash<MeshLayout>()(key.meshLayout);
            return h1 ^ (h2 << 1); // Combine the hashes
        }
    };
}

class ResourceManager {
public:
    ResourceManager();
    std::shared_ptr<Texture2D> GetTexture(const std::string& textureName);
    std::shared_ptr<CubeMapTexture> GetCubeMapTexture(const std::string& textureName);
    void BindCubeMapTexture(const std::string& name, unsigned int slot) const;

    std::shared_ptr<Mesh> GetMesh(const std::string& meshName);
    bool DeleteMesh(const std::string& meshName);

    void AddMaterial(std::string name, std::shared_ptr<Material> material)
    {
        m_MaterialManager->AddMaterial(name, material);
    }

    std::shared_ptr<Material> GetMaterial(const std::string& materialName);

    //std::shared_ptr<Model> GetModel(const std::string& modelName);
    //bool DeleteModel(const std::string& modelName);

    std::shared_ptr<Shader> GetShader(const std::string& shaderName);

    std::shared_ptr<ComputeShader> GetComputeShader(const std::string& shaderName);

    std::shared_ptr<MeshBuffer> GetMeshBuffer(const std::string& meshName, const MeshLayout& layout);
    bool DeleteMeshBuffer(const std::string& meshName, MeshLayout layout);
    void ReloadAllShaders()
    {
        m_ShaderManager->ReloadAllShaders();
    }

    void BindShader(const std::string& shaderName)
    {
        m_ShaderManager->BindShader(shaderName);
    }

    void BindMaterial(const std::string& materialName)
    {
        //check shader for compute, etc type
        m_MaterialManager->BindMaterial(materialName, m_ShaderManager->GetCurrentlyBoundShader());
    }

    void SetUniform(const std::string& uniName, UniformValue uni);

private:
    mutable std::string m_CurrentlyBoundCubeMap;
    std::unique_ptr<ShaderManager> m_ShaderManager;
    std::unique_ptr<MaterialManager> m_MaterialManager;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<MeshKey, std::shared_ptr<MeshBuffer>> m_MeshBuffers;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;

    std::unordered_map<std::string, std::array<std::string, 6>> m_TextureCubeMap
    {
        {"pisa", {
            "../assets/cube/pisa/pisa_posx.png",   
            "../assets/cube/pisa/pisa_negx.png",   
            "../assets/cube/pisa/pisa_posy.png",
            "../assets/cube/pisa/pisa_negy.png",  
            "../assets/cube/pisa/pisa_posz.png",  
            "../assets/cube/pisa/pisa_negz.png"     
        }}
    };
    std::unordered_map<std::string, std::shared_ptr<CubeMapTexture>> m_TexturesCubeMap;

    //std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
};