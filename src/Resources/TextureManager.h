#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <filesystem>
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Textures/TextureConfig.h"
#include <nlohmann/json.hpp> // Include JSON library

class TextureManager {
public:
    static TextureManager& GetInstance();

    bool LoadConfig(const std::filesystem::path& configPath);
    std::shared_ptr<ITexture> GetTexture(const std::string& name);
    void Clear();

private:
    TextureManager() = default;
    ~TextureManager() = default;

    bool Load2DTextures(const nlohmann::json& json);
    bool LoadCubeMaps(const nlohmann::json& json);
    bool LoadTextureArrays(const nlohmann::json& json);
    bool LoadComputedTextures(const nlohmann::json& json);

    std::shared_ptr<ITexture> CreateBRDFLUT(int width, int height, unsigned int numSamples);

    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_Textures;
    std::mutex m_Mutex;
};