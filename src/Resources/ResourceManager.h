#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <array>
#include <string_view>
#include <variant>

#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/CubeMapTexture.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Scene/Transform.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/Model.h"

// Uniform value types
//using UniformValue = std::variant<int, float, unsigned int, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

struct MeshKey {
    std::string name;
    MeshLayout meshLayout;

    bool operator==(const MeshKey& other) const {
        return name == other.name && meshLayout == other.meshLayout;
    }
};

namespace std {
    template <>
    struct hash<MeshKey> {
        std::size_t operator()(const MeshKey& key) const {
            std::size_t h1 = std::hash<std::string>{}(key.name);
            std::size_t h2 = std::hash<MeshLayout>{}(key.meshLayout);
            return h1 ^ (h2 << 1);
        }
    };
}

class ResourceManager {
public:
    static ResourceManager& GetInstance() {
        static ResourceManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copies
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Texture management
    std::shared_ptr<TextureBase> GetTexture(std::string_view textureName);
    std::shared_ptr<CubeMapTexture> GetCubeMapTexture(std::string_view textureName);
    void BindCubeMapTexture(std::string_view name, GLuint slot) const;

    // Mesh management
    std::shared_ptr<Mesh> GetMesh(std::string_view meshName);
    bool DeleteMesh(std::string_view meshName);

    // Model management
    std::shared_ptr<Model> GetModel(std::string_view modelName);
    bool DeleteModel(std::string_view modelName);

    // Material management
    void AddMaterial(std::string_view name, std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial(std::string_view materialName);

    // Shader management
    std::shared_ptr<Shader> GetShader(std::string_view shaderName);
    std::shared_ptr<ComputeShader> GetComputeShader(std::string_view shaderName);

    // MeshBuffer management
    std::shared_ptr<MeshBuffer> GetMeshBuffer(std::string_view meshName, const MeshLayout& layout);
    bool DeleteMeshBuffer(std::string_view meshName, const MeshLayout& layout);

    // Model MeshBuffers
    std::vector<std::shared_ptr<MeshBuffer>> GetModelMeshBuffers(std::string_view modelName, const MeshLayout& layout);
    const std::vector<MeshInfo>& GetModelMeshInfos(std::string_view modelName);

    // Shader and Material binding
    void ReloadAllShaders();
    void BindShader(std::string_view shaderName);
    void BindMaterial(std::string_view materialName);
    void SetUniform(std::string_view uniformName, const UniformValue& value);

    // Uniform and Storage Block Binding
    void RebindUniformBlocks(const std::string& shaderName);
    void RebindShaderStorageBlocks(const std::string& shaderName);

    // Access to the currently bound shader
    std::shared_ptr<BaseShader> GetCurrentlyBoundShader() const;

    // Binding Points Constants
    static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
    static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

private:
    ResourceManager();
    ~ResourceManager() = default;

    mutable std::string m_CurrentlyBoundCubeMap;
    std::unique_ptr<ShaderManager> m_ShaderManager;
    std::unique_ptr<MaterialManager> m_MaterialManager;

    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<MeshKey, std::shared_ptr<MeshBuffer>> m_MeshBuffers;
    std::unordered_map<std::string, std::filesystem::path> m_ModelPath;
    std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;

    std::unordered_map<std::string, std::array<std::filesystem::path, 6>> m_TextureCubeMapPath;
    std::unordered_map<std::string, std::shared_ptr<CubeMapTexture>> m_TexturesCubeMap;

    // Uniform Block and Shader Storage Block Binding Points
    std::unordered_map<std::string, GLuint> m_UniformBlockBindings;
    std::unordered_map<std::string, GLuint> m_ShaderStorageBlockBindings;
};