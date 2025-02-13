#pragma once
#include "GLBaseTexture.h"
#include "Bitmap.h"
#include "TextureConfig.h"
#include <array>
#include <filesystem>
#include <vector>

namespace graphics {

    class OpenGLCubeMapTexture : public GLBaseTexture
    {
    public:
        /**
         * @brief Create from 6 file paths.
         */
        OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
            const TextureConfig& config);

        /**
         * @brief Create from multiple mip levels, each level has 6 faces.
         */
        OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces,
            const TextureConfig& config);

        ~OpenGLCubeMapTexture() override = default;
    };

} // namespace graphics