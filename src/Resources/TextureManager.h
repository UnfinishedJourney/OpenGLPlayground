#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/CubeMapTexture.h"

class TextureManager {
public:
    static TextureManager& GetInstance();

    std::shared_ptr<Texture2D> GetTexture2D(const std::string& textureName);
    std::shared_ptr<CubeMapTexture> GetCubeMapTexture(const std::string& cubeMapName);
    std::shared_ptr<Texture2D> GetHeightMap(const std::string& heightMapName);

    void RegisterTexture2D(const std::string& name, std::shared_ptr<Texture2D> texture);
    void InitializeBRDFLUT(int width = 256, int height = 256, unsigned int numSamples = 1024);

    void Clear();

private:
    TextureManager();
    ~TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Texture2Ds;
    std::unordered_map<std::string, std::shared_ptr<CubeMapTexture>> m_CubeMapTextures;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_HeightMaps;

    std::unordered_map<std::string, std::filesystem::path> m_TexturePaths;
    std::unordered_map<std::string, std::array<std::filesystem::path, 6>> m_CubeMapTexturePaths;

    mutable std::mutex m_Mutex;

    std::filesystem::path GetTexturePath(const std::string& textureName) const;
    std::array<std::filesystem::path, 6> GetCubeMapPaths(const std::string& cubeMapName) const;
};