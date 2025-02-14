#pragma once
#include "GLBaseTexture.h"
#include "TextureConfig.h"
#include <array>
#include <filesystem>
#include <vector>

namespace graphics {

    class OpenGLCubeMapTexture : public GLBaseTexture {
    public:
        OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces, const TextureConfig& config);
        OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces, const TextureConfig& config);
        ~OpenGLCubeMapTexture() override = default;
    };

} // namespace graphics