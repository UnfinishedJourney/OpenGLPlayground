#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <array>
#include <nlohmann/json.hpp>
#include "ITexture.h"
#include "TextureConfig.h"

namespace graphics {

    class Bitmap;

    class TextureManager {
    public:
        static TextureManager& GetInstance();
        bool LoadConfig(const std::filesystem::path& configPath);
        std::shared_ptr<ITexture> GetTexture(const std::string& name);

        std::shared_ptr<ITexture> Load2DTexture(const std::string& name, const std::string& path, const TextureConfig& config);
        std::shared_ptr<ITexture> LoadCubeMapTexture(const std::string& name, const std::array<std::string, 6>& facePaths, const TextureConfig& config);
        std::shared_ptr<ITexture> LoadTextureArray(const std::string& name, const std::string& path, const TextureConfig& config, uint32_t totalFrames, uint32_t gridX, uint32_t gridY);
        std::shared_ptr<ITexture> CreateBRDFTexture(const std::string& name, int width, int height, unsigned int numSamples);
        bool ConvertAndLoadEquirectHDR(const std::string& cubeMapName, const std::string& equirectPath);
        void Clear();

    private:
        TextureManager(const std::filesystem::path& configPath = "../configs/textures_config.json");
        ~TextureManager() = default;
        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        bool Load2DTextures(const nlohmann::json& json);
        bool LoadCubeMaps(const nlohmann::json& json);
        bool LoadComputedTextures(const nlohmann::json& json);

        std::string ToLower(const std::string& str);
        bool DetermineSRGB(const std::string& pathStr);
        TextureConfig MakeSomeCubeMapConfig(bool isHDR);
        void SaveFacesToDisk(const Bitmap& cubeMap, const std::array<std::filesystem::path, 6>& facePaths, const std::string& prefix);

        std::unordered_map<std::string, std::shared_ptr<ITexture>> textures_;
    };

} // namespace graphics