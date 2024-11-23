#pragma once

#include "Graphics/Textures/TextureBase.h"
#include <array>
#include <string_view>
#include <filesystem>

class CubeMapTexture : public TextureBase {
public:
    // Constructor for loading from six separate image paths
    CubeMapTexture(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps = true);

    // Move semantics
    CubeMapTexture(CubeMapTexture&& other) noexcept = default;
    CubeMapTexture& operator=(CubeMapTexture&& other) noexcept = default;

    // Delete copy semantics
    CubeMapTexture(const CubeMapTexture& other) = delete;
    CubeMapTexture& operator=(const CubeMapTexture& other) = delete;

    void Bind(GLuint slot = 0) const override;
    void Unbind(GLuint slot = 0) const override;

private:
    void LoadCubeMap(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps);
};