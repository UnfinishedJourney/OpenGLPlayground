#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <unordered_map>
#include <filesystem>

static const std::unordered_map<std::string, std::filesystem::path> G_Texture_Path{
    {"cuteDog", "../assets/cute_dog.png"},
    {"heightmap", "../assets/heightmap.png"},
    {"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"},
    {"damagedHelmet1", "../assets/DamagedHelmet/glTF/Default_albedo.jpg"},
    {"damagedHelmet2", "../assets/DamagedHelmet/glTF/Default_AO.jpg"},
    {"damagedHelmet3", "../assets/DamagedHelmet/glTF/Default_emissive.jpg"}
};

static const std::unordered_map<std::string, std::array<std::filesystem::path, 6>> G_CubeMap_Texture_Path{
    {"pisa", {
        "../assets/cube/pisa/pisa_posx.png",
        "../assets/cube/pisa/pisa_negx.png",
        "../assets/cube/pisa/pisa_posy.png",
        "../assets/cube/pisa/pisa_negy.png",
        "../assets/cube/pisa/pisa_posz.png",
        "../assets/cube/pisa/pisa_negz.png"
    }}
};

std::shared_ptr<Material> MaterialManager::GetMaterial(std::string_view materialName) {
    auto it = m_Materials.find(std::string(materialName));
    if (it != m_Materials.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->warn("Material '{}' not found.", materialName);
        return nullptr;
    }
}

std::shared_ptr<Texture2D> MaterialManager::GetTexture2D(std::string_view textureName) {
    if (textureName.empty()) {
        return nullptr;
    }

    auto it = m_Texture2Ds.find(std::string(textureName));
    if (it != m_Texture2Ds.end()) {
        return it->second;
    }

    std::shared_ptr<Texture2D> texture;
    auto pathIt = G_Texture_Path.find(std::string(textureName));
    if (pathIt != G_Texture_Path.end()) {
        texture = std::make_shared<Texture2D>(pathIt->second);
    }
    else {
        texture = std::make_shared<Texture2D>(std::filesystem::path(textureName));
    }

    m_Texture2Ds[std::string(textureName)] = texture;
    return texture;
}

std::shared_ptr<CubeMapTexture> MaterialManager::GetCubeMapTexture(std::string_view textureName) {
    if (textureName.empty()) {
        return nullptr;
    }

    auto it = m_CubeMapTextures.find(std::string(textureName));
    if (it != m_CubeMapTextures.end()) {
        return it->second;
    }

    auto pathIt = G_CubeMap_Texture_Path.find(std::string(textureName));
    if (pathIt != G_CubeMap_Texture_Path.end()) {
        const auto& facePaths = pathIt->second;
        auto texture = std::make_shared<CubeMapTexture>(facePaths);
        m_CubeMapTextures[std::string(textureName)] = texture;
        return texture;
    }
    else {
        Logger::GetLogger()->error("CubeMapTexture '{}' not found in predefined paths.", textureName);
        return nullptr;
    }
}

void MaterialManager::AddMaterial(std::string_view name, const std::shared_ptr<Material>& material) {
    m_Materials.emplace(std::string(name), material);
}

void MaterialManager::BindMaterial(std::string_view name, const std::shared_ptr<BaseShader>& shader) {
    auto logger = Logger::GetLogger();
    std::string materialName(name);

    static GLuint lastShaderID = currentShaderID;
    currentShaderID = shader->GetRendererID();

    auto materialIt = m_Materials.find(materialName);
    if (materialIt == m_Materials.end()) {
        logger->error("Material '{}' not found. Cannot bind.", materialName);
        return;
    }

    try {
        materialIt->second->Bind(shader);
        m_CurrentlyBoundMaterial = materialName;
        lastShaderID = currentShaderID;
        logger->debug("Material '{}' bound successfully to shader ID {}.", materialName, currentShaderID);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind material '{}': {}", materialName, e.what());
    }
}

void MaterialManager::UnbindMaterial() {
    if (m_CurrentlyBoundMaterial.empty()) {
        return;
    }

    auto materialIt = m_Materials.find(m_CurrentlyBoundMaterial);
    if (materialIt != m_Materials.end()) {
        materialIt->second->Unbind();
        Logger::GetLogger()->debug("Material '{}' unbound successfully.", m_CurrentlyBoundMaterial);
    }

    m_CurrentlyBoundMaterial.clear();
}