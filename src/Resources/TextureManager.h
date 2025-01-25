#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "Graphics/Textures/ITexture.h"
#include "Graphics/Textures/TextureConfig.h"

class TextureManager {
public:
    // Singleton
    static TextureManager& GetInstance();
    TextureManager(const std::filesystem::path configPath = "../assets/resource_paths.json");
    // Load from JSON config
    bool LoadConfig(const std::filesystem::path& configPath);

    // Retrieve a texture by name
    std::shared_ptr<ITexture> GetTexture(const std::string& name);

    // Explicitly load a single texture by path (2D only)
    std::shared_ptr<ITexture> LoadTexture(const std::string& name, const std::string& path);

    // Clear all textures from memory
    void Clear();

private:
    ~TextureManager() = default;

    // Helpers
    bool Load2DTextures(const nlohmann::json& json);
    bool LoadCubeMaps(const nlohmann::json& json);
    bool LoadTextureArrays(const nlohmann::json& json);
    bool LoadComputedTextures(const nlohmann::json& json);

    // Optionally implement specialized computed textures (e.g., BRDF LUT)
    std::shared_ptr<ITexture> CreateBRDFLUT(int width, int height, unsigned int numSamples);

    // Storage
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_Textures;
    std::mutex m_Mutex;
};