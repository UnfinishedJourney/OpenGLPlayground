#pragma once

#include "Graphics/Textures/TextureBase.h"
#include <array>
#include <string_view>

class CubeMapTexture : public TextureBase {
public:
    // Constructor for loading from six separate image paths
    CubeMapTexture(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps = true);

    ~CubeMapTexture() override = default;

    CubeMapTexture(const CubeMapTexture& other) = delete;
    CubeMapTexture& operator=(const CubeMapTexture& other) = delete;
    CubeMapTexture(CubeMapTexture&& other) noexcept = default;
    CubeMapTexture& operator=(CubeMapTexture&& other) noexcept = default;

    void Bind(GLuint slot = 0) const override;
    void Unbind(GLuint slot = 0) const override;

private:
    void LoadCubeMap(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps);
};