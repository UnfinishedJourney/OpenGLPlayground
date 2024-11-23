#include "Resources/TextureManager.h"
#include "Utilities/Logger.h"

#include <stdexcept>

TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

TextureManager::TextureManager() {
    m_TexturePaths = {
        {"cuteDog", "../assets/cute_dog.png"},
        {"heightmap", "../assets/heightmap.png"},
        {"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"},
        {"damagedHelmet1", "../assets/DamagedHelmet/glTF/Default_albedo.jpg"},
        {"damagedHelmet2", "../assets/DamagedHelmet/glTF/Default_AO.jpg"},
        {"damagedHelmet3", "../assets/DamagedHelmet/glTF/Default_emissive.jpg"}
    };

    m_CubeMapTexturePaths = {
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

std::filesystem::path TextureManager::GetTexturePath(const std::string& textureName) const {
    auto it = m_TexturePaths.find(textureName);
    if (it != m_TexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("Texture name '{}' not found in texture path mappings.", textureName);
        return {};
    }
}

std::array<std::filesystem::path, 6> TextureManager::GetCubeMapPaths(const std::string& cubeMapName) const {
    auto it = m_CubeMapTexturePaths.find(cubeMapName);
    if (it != m_CubeMapTexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("CubeMap name '{}' not found in cube map texture path mappings.", cubeMapName);
        return {};
    }
}

std::shared_ptr<Texture2D> TextureManager::GetTexture2D(const std::string& textureName) {
    std::lock_guard<std::mutex> lock(m_Mutex); // Ensure thread-safe access

    auto it = m_Texture2Ds.find(textureName);
    if (it != m_Texture2Ds.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(textureName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Texture file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    try {
        auto texture = std::make_shared<Texture2D>(texturePath);
        m_Texture2Ds.emplace(textureName, texture);
        return texture;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading Texture2D '{}': {}", textureName, e.what());
        return nullptr;
    }
}

std::shared_ptr<CubeMapTexture> TextureManager::GetCubeMapTexture(const std::string& cubeMapName) {
    std::lock_guard<std::mutex> lock(m_Mutex); 

    auto it = m_CubeMapTextures.find(cubeMapName);
    if (it != m_CubeMapTextures.end()) {
        return it->second;
    }

    auto facePaths = GetCubeMapPaths(cubeMapName);
    if (facePaths.empty()) {
        Logger::GetLogger()->error("Cube map face paths for '{}' are empty.", cubeMapName);
        return nullptr;
    }

    for (const auto& path : facePaths) {
        if (!std::filesystem::exists(path)) {
            Logger::GetLogger()->error("Cube map face file '{}' does not exist.", path.string());
            return nullptr;
        }
    }

    try {
        auto cubeMap = std::make_shared<CubeMapTexture>(facePaths);
        m_CubeMapTextures.emplace(cubeMapName, cubeMap);
        return cubeMap;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading CubeMapTexture '{}': {}", cubeMapName, e.what());
        return nullptr;
    }
}

std::shared_ptr<Texture2D> TextureManager::GetHeightMap(const std::string& heightMapName) {
    std::lock_guard<std::mutex> lock(m_Mutex); // Ensure thread-safe access

    auto it = m_HeightMaps.find(heightMapName);
    if (it != m_HeightMaps.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(heightMapName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Heightmap file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    try {
        auto heightMap = std::make_shared<Texture2D>(texturePath);
        m_HeightMaps.emplace(heightMapName, heightMap);
        return heightMap;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading HeightMap '{}': {}", heightMapName, e.what());
        return nullptr;
    }
}

void TextureManager::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex); 

    m_Texture2Ds.clear();
    m_CubeMapTextures.clear();
    m_HeightMaps.clear();

    Logger::GetLogger()->info("Cleared all textures from TextureManager.");
}