#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"

#include <unordered_map>
#include <filesystem>

static std::unordered_map<std::string, std::filesystem::path> G_Texture_Path{
    {"cuteDog", "../assets/cute_dog.png"},
    {"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"},
    {"damagedHelmet1", "../assets/DamagedHelmet/glTF/Default_albedo.jpg"},
    {"damagedHelmet2", "../assets/DamagedHelmet/glTF/Default_AO.jpg"},
    {"damagedHelmet3", "../assets/DamagedHelmet/glTF/Default_emissive.jpg"}
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

std::shared_ptr<Texture2D> MaterialManager::GetTexture(std::string_view textureName) {
    if (textureName.empty()) {
        return nullptr;
    }

    auto it = m_Textures.find(std::string(textureName));
    if (it != m_Textures.end()) {
        return it->second;
    }

    std::shared_ptr<Texture2D> texture;
    auto pathIt = G_Texture_Path.find(std::string(textureName));
    if (pathIt != G_Texture_Path.end()) {
        texture = std::make_shared<Texture2D>(pathIt->second);
    }
    else {
        texture = std::make_shared<Texture2D>(textureName);
    }

    m_Textures[std::string(textureName)] = texture;
    return texture;
}

void MaterialManager::AddMaterial(std::string_view name, std::shared_ptr<Material> material) {
    m_Materials[std::string(name)] = std::move(material);
}

void MaterialManager::BindMaterial(std::string_view name, std::shared_ptr<BaseShader> shader) {
    auto logger = Logger::GetLogger();

    if (name == m_CurrentlyBoundMaterial) {
        logger->debug("Material '{}' is already bound. Skipping bind.", name);
        return;
    }

    auto materialIt = m_Materials.find(std::string(name));
    if (materialIt == m_Materials.end()) {
        logger->error("Material '{}' not found. Cannot bind.", name);
        return;
    }

    try {
        materialIt->second->Bind(shader);
        m_CurrentlyBoundMaterial = name;
        logger->info("Material '{}' bound successfully.", name);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind material '{}': {}", name, e.what());
    }
}

void MaterialManager::UnbindMaterial(std::string_view name) {
    // Implement unbind logic if necessary
}