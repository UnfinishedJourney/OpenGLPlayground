#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"

#include <glad/glad.h>
#include <fstream>
#include <nlohmann/json.hpp>

std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& materialName)
{
    return m_Materials[materialName];
}

std::unordered_map<std::string, std::string> G_Texture_Path
{
    {"cuteDog", "../assets/cute_dog.png"},
    {"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"},

    {"damagedHelmet1", "../assets/DamagedHelmet/glTF/Default_albedo.jpg"},
    {"damagedHelmet2", "../assets/DamagedHelmet/glTF/Default_AO.jpg"},
    {"damagedHelmet3", "../assets/DamagedHelmet/glTF/Default_emissive.jpg"}

};

std::shared_ptr<Texture2D> MaterialManager::GetTexture(const std::string& textureName)
{
    if (textureName.empty())
        return nullptr;

    if (m_Textures.find(textureName) != m_Textures.end()) {
        return m_Textures[textureName];
    }

    std::shared_ptr<Texture2D> texture;
    if (G_Texture_Path.find(textureName) != G_Texture_Path.end())
        texture = std::make_shared<Texture2D>(G_Texture_Path[textureName]);
    else
        texture = std::make_shared<Texture2D>(textureName);

    m_Textures[textureName] = texture;
    return texture;
}

void MaterialManager::AddMaterial(std::string name, std::shared_ptr<Material> material)
{
	m_Materials[name] = material;
}

//need to check if the shader is the same and bind through shader
void MaterialManager::BindMaterial(const std::string& name, std::shared_ptr<BaseShader> shader)
{
    auto logger = Logger::GetLogger();

    if (name == m_CurrentlyBoundMaterial) {
        logger->debug("Material '{}' is already bound. Skipping bind.", name);
        return;
    }

    auto materialIt = m_Materials.find(name);
    if (materialIt == m_Materials.end()) {
        logger->error("materials '{}' not found. Cannot bind.", name);
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

//need to add unbind
void MaterialManager::UnbindMaterial(const std::string& name)
{
}
