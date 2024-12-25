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
    // Singleton access method
    static TextureManager& GetInstance();

    // Load textures based on a JSON configuration file
    bool LoadConfig(const std::filesystem::path& configPath);

    // Retrieve a texture by name
    std::shared_ptr<ITexture> GetTexture(const std::string& name);

    // **New Method**: Load a texture by name and filepath
    std::shared_ptr<ITexture> LoadTexture(const std::string& name, const std::string& path);

    // Clear all loaded textures
    void Clear();

private:
    // Private constructor and destructor for Singleton pattern
    TextureManager() = default;
    ~TextureManager() = default;

    // Helper methods for different texture types
    bool Load2DTextures(const nlohmann::json& json);
    bool LoadCubeMaps(const nlohmann::json& json);
    bool LoadTextureArrays(const nlohmann::json& json);
    bool LoadComputedTextures(const nlohmann::json& json);

    // Method to create BRDF Lookup Texture (LUT)
    std::shared_ptr<ITexture> CreateBRDFLUT(int width, int height, unsigned int numSamples);

    // Texture storage with thread safety
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_Textures;
    std::mutex m_Mutex;
};