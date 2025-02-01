#pragma once

#include "ITexture.h"
#include "TextureConfig.h"
#include <array>
#include <filesystem>
#include <vector>
#include <glad/glad.h>

/**
 * @brief An OpenGL cube map texture, possibly with HDR or sRGB.
 */
class OpenGLCubeMapTexture : public ITexture {
public:
    /**
     * @brief Constructs a cube map texture from 6 file paths (one for each face).
     */
    OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
        TextureConfig config);

    /**
     * @brief Constructs a cube map texture with multiple mip levels.
     * @param mipFaces A vector where each element is an array of 6 file paths corresponding to one mip level.
     * @param config Texture creation parameters.
     */
    OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mipFaces,
        TextureConfig config);

    ~OpenGLCubeMapTexture() override;

    void Bind(uint32_t unit) const override;
    void Unbind(uint32_t unit) const override;

    uint32_t GetWidth()  const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }

    uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
    bool     IsBindless()        const override { return m_IsBindless; }

private:
    GLuint   m_TextureID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint64_t m_BindlessHandle = 0;
    bool     m_IsBindless = false;
};