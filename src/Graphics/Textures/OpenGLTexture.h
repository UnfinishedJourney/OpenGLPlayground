#pragma once

#include "ITexture.h"
#include "TextureData.h"
#include "TextureConfig.h"
#include <glad/glad.h>
#include <stdexcept>
#include <cstdint>

/**
 * @brief Represents a 2D texture in OpenGL, optionally bindless.
 *
 * It can handle either LDR (8-bit) or HDR (float) data,
 * as well as sRGB if desired.
 */
class OpenGLTexture : public ITexture {
public:
    /**
     * @param data   The image data (either float or byte).
     * @param config Creation parameters (internal format, mipmaps, etc.).
     */
    OpenGLTexture(const TextureData& data, TextureConfig config);
    ~OpenGLTexture() override;

    void Bind(uint32_t unit) const override;
    void Unbind(uint32_t unit) const override;

    uint32_t GetWidth()  const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }

    uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
    bool     IsBindless()        const override { return m_IsBindless; }

    [[nodiscard]] GLuint GetTextureID() const { return m_TextureID; }

private:
    GLuint   m_TextureID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint64_t m_BindlessHandle = 0;
    bool     m_IsBindless = false;

    /**
     * @brief Choose an appropriate internalFormat if the user wants sRGB or HDR.
     */
    static GLenum ResolveInternalFormat(const TextureConfig& config, int channels);
};