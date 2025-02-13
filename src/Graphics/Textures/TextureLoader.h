#pragma once
#include <memory>
#include <string>
#include <array>
#include "ITexture.h"
#include "TextureConfig.h"

namespace graphics {

    /**
     * @brief Low-level loader that creates textures from disk data or computed results.
     *
     * All OpenGL–specific code is contained here (or in the concrete classes).
     */
    class TextureLoader {
    public:
        /// Loads a 2D texture from file using the given configuration.
        static std::shared_ptr<ITexture> Load2DTexture(const std::string& filePath, const TextureConfig& config);

        /// Loads a cube map texture from an array of 6 file paths.
        static std::shared_ptr<ITexture> LoadCubeMapTexture(const std::array<std::string, 6>& facePaths, const TextureConfig& config);

        /// Loads a texture array (sprite sheet) from file.
        static std::shared_ptr<ITexture> LoadTextureArray(const std::string& filePath,
            const TextureConfig& config,
            uint32_t totalFrames,
            uint32_t gridX,
            uint32_t gridY);

        /// Creates a computed BRDF LUT texture using a compute shader.
        static std::shared_ptr<ITexture> CreateBRDFTexture(int width, int height, unsigned int numSamples);
    };

} // namespace graphics