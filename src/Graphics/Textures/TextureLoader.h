#pragma once
#include <memory>
#include <string>
#include <array>
#include "ITexture.h"
#include "TextureConfig.h"

namespace graphics {

    class TextureLoader {
    public:
        static std::shared_ptr<ITexture> Load2DTexture(const std::string& filePath, const TextureConfig& config);
        static std::shared_ptr<ITexture> LoadCubeMapTexture(const std::array<std::string, 6>& facePaths, const TextureConfig& config);
        static std::shared_ptr<ITexture> LoadTextureArray(const std::string& filePath, const TextureConfig& config,
            uint32_t totalFrames, uint32_t gridX, uint32_t gridY);
        static std::shared_ptr<ITexture> CreateBRDFTexture(int width, int height, unsigned int numSamples);
    };

} // namespace graphics