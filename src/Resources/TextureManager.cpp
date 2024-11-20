#include "Resources/TextureManager.h"
#include "Utilities/Logger.h"

TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

TextureManager::TextureManager() {
    // Initialize your custom texture paths
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

std::filesystem::path TextureManager::GetTexturePath(std::string_view textureName) {
    auto it = m_TexturePaths.find(std::string(textureName));
    if (it != m_TexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("Texture name '{}' not found in texture path mappings.", textureName);
        return {};
    }
}

std::array<std::filesystem::path, 6> TextureManager::GetCubeMapPaths(std::string_view cubeMapName) {
    auto it = m_CubeMapTexturePaths.find(std::string(cubeMapName));
    if (it != m_CubeMapTexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("CubeMap name '{}' not found in cube map texture path mappings.", cubeMapName);
        return {};
    }
}

std::shared_ptr<Texture2D> TextureManager::GetTexture2D(std::string_view textureName) {
    auto it = m_Texture2Ds.find(std::string(textureName));
    if (it != m_Texture2Ds.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(textureName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Texture file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    auto texture = std::make_shared<Texture2D>(texturePath);
    m_Texture2Ds[std::string(textureName)] = texture;
    return texture;
}

std::shared_ptr<CubeMapTexture> TextureManager::GetCubeMapTexture(std::string_view textureName) {
    auto it = m_CubeMapTextures.find(std::string(textureName));
    if (it != m_CubeMapTextures.end()) {
        return it->second;
    }

    auto facePaths = GetCubeMapPaths(textureName);
    if (facePaths.empty()) {
        Logger::GetLogger()->error("Cube map face paths for '{}' are empty.", textureName);
        return nullptr;
    }

    // Check if all face paths exist
    for (const auto& path : facePaths) {
        if (!std::filesystem::exists(path)) {
            Logger::GetLogger()->error("Cube map face file '{}' does not exist.", path.string());
            return nullptr;
        }
    }

    auto cubeMap = std::make_shared<CubeMapTexture>(facePaths);
    m_CubeMapTextures[std::string(textureName)] = cubeMap;
    return cubeMap;
}

std::shared_ptr<Texture2D> TextureManager::GetHeightMap(std::string_view textureName) {
    auto it = m_HeightMaps.find(std::string(textureName));
    if (it != m_HeightMaps.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(textureName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Heightmap file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    auto heightMap = std::make_shared<Texture2D>(texturePath);
    m_HeightMaps[std::string(textureName)] = heightMap;
    return heightMap;
}

void TextureManager::Clear() {
    m_Texture2Ds.clear();
    m_CubeMapTextures.clear();
    m_HeightMaps.clear();
}