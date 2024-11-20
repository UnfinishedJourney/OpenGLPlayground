#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <string_view>
#include <array>

#include "Graphics/Textures/TextureBase.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/CubeMapTexture.h"

class TextureManager {
public:
    static TextureManager& GetInstance();

    std::shared_ptr<Texture2D> GetTexture2D(std::string_view textureName);
    std::shared_ptr<CubeMapTexture> GetCubeMapTexture(std::string_view textureName);
    std::shared_ptr<Texture2D> GetHeightMap(std::string_view textureName);

    void Clear();

private:
    TextureManager();
    ~TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Texture2Ds;
    std::unordered_map<std::string, std::shared_ptr<CubeMapTexture>> m_CubeMapTextures;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_HeightMaps;

    // Custom texture path mappings
    std::unordered_map<std::string, std::filesystem::path> m_TexturePaths;
    std::unordered_map<std::string, std::array<std::filesystem::path, 6>> m_CubeMapTexturePaths;

    // Helper functions
    std::filesystem::path GetTexturePath(std::string_view textureName);
    std::array<std::filesystem::path, 6> GetCubeMapPaths(std::string_view cubeMapName);
};