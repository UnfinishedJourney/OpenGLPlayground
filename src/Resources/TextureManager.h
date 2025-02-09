#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <filesystem>
#include <array>
#include <nlohmann/json.hpp>
#include "Graphics/Textures/ITexture.h"

namespace Graphics {

    struct TextureConfig; // Forward if defined elsewhere
    class Bitmap;

    /**
     * @brief Singleton manager for loading and retrieving textures.
     *
     * Supports 2D textures, cube maps (from 6–face definitions or single equirect HDR
     * converted into faces), texture arrays, and computed textures such as BRDF LUTs.
     */
    class TextureManager {
    public:
        static TextureManager& GetInstance();

        explicit TextureManager(const std::filesystem::path& configPath = "../assets/resource_paths.json");
        bool LoadConfig(const std::filesystem::path& configPath);
        std::shared_ptr<ITexture> GetTexture(const std::string& name);
        std::shared_ptr<ITexture> LoadTexture(const std::string& name, const std::string& path);
        void Clear();

    private:
        ~TextureManager() = default;
        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        bool Load2DTextures(const nlohmann::json& json);
        bool LoadCubeMaps(const nlohmann::json& json);
        bool LoadTextureArrays(const nlohmann::json& json);
        bool LoadComputedTextures(const nlohmann::json& json);
        bool ConvertAndLoadEquirectHDR(const std::string& cubeMapName, const std::string& equirectPath);
        std::shared_ptr<ITexture> CreateBRDFLUT(int width, int height, unsigned int numSamples);

        std::string ToLower(const std::string& str);
        bool IsHDRTexture(const std::filesystem::path& path);
        bool DetermineSRGB(const std::string& pathStr);
        void SaveFacesToDisk(const Bitmap& cubeMap,
            const std::array<std::filesystem::path, 6>& facePaths,
            const std::string& prefix);

        // A simple helper for cube map config.
        TextureConfig MakeSomeCubeMapConfig(bool isHDR);

        std::unordered_map<std::string, std::shared_ptr<ITexture>> textures_;
    };

} // namespace Graphics