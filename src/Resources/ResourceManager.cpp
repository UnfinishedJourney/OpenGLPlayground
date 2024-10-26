#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/Cube.h"

ResourceManager::ResourceManager()
    : m_CurrentlyBoundCubeMap("") {
    m_ShaderManager = std::make_unique<ShaderManager>("../shaders/metadata.json", "../shaders/config.json");
    m_MaterialManager = std::make_unique<MaterialManager>();

    // Initialize CubeMap textures
    m_TextureCubeMap = {
        {"pisa", {
            "../assets/cube/pisa/pisa_posx.png",
            "../assets/cube/pisa/pisa_negx.png",
            "../assets/cube/pisa/pisa_posy.png",
            "../assets/cube/pisa/pisa_negy.png",
            "../assets/cube/pisa/pisa_posz.png",
            "../assets/cube/pisa/pisa_negz.png"
        }}
    };
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(std::string_view textureName) {
    return m_MaterialManager->GetTexture(textureName);
}

std::shared_ptr<CubeMapTexture> ResourceManager::GetCubeMapTexture(std::string_view name) {
    if (name.empty()) {
        return nullptr;
    }

    auto it = m_TexturesCubeMap.find(std::string(name));
    if (it != m_TexturesCubeMap.end()) {
        return it->second;
    }

    auto texIt = m_TextureCubeMap.find(std::string(name));
    if (texIt == m_TextureCubeMap.end()) {
        Logger::GetLogger()->error("CubeMapTexture not found: '{}'", name);
        return nullptr;
    }

    const auto& facePaths = texIt->second;

    try {
        auto cubeMapTexture = std::make_shared<CubeMapTexture>(facePaths);
        m_TexturesCubeMap[std::string(name)] = cubeMapTexture;
        return cubeMapTexture;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Failed to load CubeMapTexture '{}': {}", name, e.what());
        return nullptr;
    }
}

void ResourceManager::BindCubeMapTexture(std::string_view name, GLuint slot) const {
    if (m_CurrentlyBoundCubeMap == name) {
        return;
    }

    auto it = m_TexturesCubeMap.find(std::string(name));
    if (it != m_TexturesCubeMap.end()) {
        auto cubeMap = it->second;
        if (cubeMap) {
            cubeMap->Bind(slot);
            m_CurrentlyBoundCubeMap = name;
        }
        else {
            throw std::runtime_error("CubeMapTexture '" + std::string(name) + "' not found.");
        }
    }
    else {
        throw std::runtime_error("CubeMapTexture '" + std::string(name) + "' not found.");
    }
}

void ResourceManager::SetUniform(std::string_view uniformName, const UniformValue& value) {
    auto shader = m_ShaderManager->GetCurrentlyBoundShader();
    if (shader) {
        std::visit([&](auto&& arg) {
            shader->SetUniform(uniformName, arg);
            Logger::GetLogger()->debug("Set uniform '{}' with value.", uniformName);
            }, value);
    }
    else {
        Logger::GetLogger()->error("No shader is currently bound. Cannot set uniform '{}'.", uniformName);
    }
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(std::string_view meshName) {
    auto it = m_Meshes.find(std::string(meshName));
    if (it != m_Meshes.end()) {
        return it->second;
    }

    // Create mesh if not found
    std::shared_ptr<Mesh> mesh;
    if (meshName == "cube") {
        mesh = std::make_shared<Cube>();
    }
    else {
        Logger::GetLogger()->warn("Mesh '{}' not found. Returning nullptr.", meshName);
        return nullptr;
    }

    m_Meshes[std::string(meshName)] = mesh;
    return mesh;
}

bool ResourceManager::DeleteMesh(std::string_view meshName) {
    return m_Meshes.erase(std::string(meshName)) > 0;
}

void ResourceManager::AddMaterial(std::string_view name, std::shared_ptr<Material> material) {
    m_MaterialManager->AddMaterial(name, std::move(material));
}

std::shared_ptr<Material> ResourceManager::GetMaterial(std::string_view materialName) {
    return m_MaterialManager->GetMaterial(materialName);
}

std::shared_ptr<Shader> ResourceManager::GetShader(std::string_view shaderName) {
    return m_ShaderManager->GetShader(shaderName);
}

std::shared_ptr<ComputeShader> ResourceManager::GetComputeShader(std::string_view shaderName) {
    return m_ShaderManager->GetComputeShader(shaderName);
}

std::shared_ptr<MeshBuffer> ResourceManager::GetMeshBuffer(std::string_view meshName, const MeshLayout& mLayout) {
    MeshKey key = { std::string(meshName), mLayout };

    auto it = m_MeshBuffers.find(key);
    if (it != m_MeshBuffers.end()) {
        return it->second;
    }

    auto mesh = GetMesh(meshName);
    if (!mesh) {
        Logger::GetLogger()->error("Mesh '{}' not found. Cannot create MeshBuffer.", meshName);
        return nullptr;
    }

    auto meshBuffer = std::make_shared<MeshBuffer>(*mesh, mLayout);
    m_MeshBuffers[key] = meshBuffer;
    return meshBuffer;
}

bool ResourceManager::DeleteMeshBuffer(std::string_view meshName, const MeshLayout& mLayout) {
    MeshKey key = { std::string(meshName), mLayout };
    return m_MeshBuffers.erase(key) > 0;
}

void ResourceManager::ReloadAllShaders() {
    m_ShaderManager->ReloadAllShaders();
}

void ResourceManager::BindShader(std::string_view shaderName) {
    m_ShaderManager->BindShader(shaderName);
}

void ResourceManager::BindMaterial(std::string_view materialName) {
    auto shader = m_ShaderManager->GetCurrentlyBoundShader();
    if (shader) {
        m_MaterialManager->BindMaterial(materialName, shader);
    }
    else {
        Logger::GetLogger()->error("No shader is currently bound. Cannot bind material '{}'.", materialName);
    }
}