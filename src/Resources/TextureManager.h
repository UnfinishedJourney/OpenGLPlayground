#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <filesystem>
#include <nlohmann/json.hpp>

// Forward declarations
class ITexture;
struct TextureConfig;

// TextureManager Class Definition
/**
 * @brief A singleton manager for loading, storing, and retrieving textures.
 *
 * Supports 2D textures, cube maps, array textures, and computed textures.
 */
class TextureManager {
public:
    // Singleton Access
    static TextureManager& GetInstance();

    /**
     * @param configPath The JSON file containing texture definitions (optional).
     */
    explicit TextureManager(const std::filesystem::path& configPath = "../assets/resource_paths.json");

    /**
     * @brief Load the texture definitions from a JSON file.
     * @return True on success, false on failure.
     */
    bool LoadConfig(const std::filesystem::path& configPath);

    /**
     * @brief Retrieve a texture by name (as specified in the JSON or manually loaded).
     */
    std::shared_ptr<ITexture> GetTexture(const std::string& name);

    /**
     * @brief Explicitly load a single 2D texture by path and store it under `name`.
     * @return The created texture, or nullptr on failure.
     */
    std::shared_ptr<ITexture> LoadTexture(const std::string& name, const std::string& path);

    /**
     * @brief Remove all textures from the cache.
     */
    void Clear();

private:
    // Private Destructor
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // Loading Functions
    bool Load2DTextures(const nlohmann::json& json);
    bool LoadCubeMaps(const nlohmann::json& json);
    bool LoadTextureArrays(const nlohmann::json& json);
    bool LoadComputedTextures(const nlohmann::json& json);

    /**
     * @brief Create a BRDF LUT texture via compute shader.
     */
    std::shared_ptr<ITexture> CreateBRDFLUT(int width, int height, unsigned int numSamples);

    // Utility Functions
    std::string ToLower(const std::string& str);
    bool IsHDRTexture(const std::filesystem::path& path);
    bool DetermineSRGB(const std::string& pathStr);

private:
    // Texture Storage
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_Textures;
};