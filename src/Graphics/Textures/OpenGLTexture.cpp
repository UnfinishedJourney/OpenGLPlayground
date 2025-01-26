#include "OpenGLTexture.h"
#include <cmath>
#include <stdexcept>

OpenGLTexture::OpenGLTexture(const TextureData& data, const TextureConfig& config)
{
    m_Width = data.GetWidth();
    m_Height = data.GetHeight();

    // 1) Create a GL texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
    if (!m_TextureID) {
        throw std::runtime_error("Failed to create GL texture object.");
    }

    // 2) Allocate storage (with mip levels if requested)
    int levels = config.generateMips
        ? static_cast<int>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1
        : 1;

    glTextureStorage2D(m_TextureID, levels, config.internalFormat, m_Width, m_Height);

    // 3) Upload the data into the base level
    glTextureSubImage2D(m_TextureID, 0, 0, 0,
        m_Width, m_Height,
        GL_RGBA, GL_UNSIGNED_BYTE,
        data.GetData());

    // 4) Generate MIPs if requested
    if (config.generateMips) {
        glGenerateTextureMipmap(m_TextureID);
    }

    // 5) Set filters and wrapping
    glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT);

    // 6) Optional anisotropy
    if (config.useAnisotropy) {
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
    }

    // 7) Optional bindless
    if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
        m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
        if (m_BindlessHandle) {
            glMakeTextureHandleResidentARB(m_BindlessHandle);
            m_IsBindless = true;
        }
    }
}

OpenGLTexture::~OpenGLTexture()
{
    // Clean up bindless if used
    if (m_IsBindless && m_BindlessHandle) {
        glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    }
    // Delete texture
    if (m_TextureID) {
        glDeleteTextures(1, &m_TextureID);
    }
}

void OpenGLTexture::Bind(uint32_t unit) const
{
    // For non-bindless, just bind
    if (!m_IsBindless) {
        glBindTextureUnit(unit, m_TextureID);
    }
}

void OpenGLTexture::Unbind(uint32_t unit) const
{
    if (!m_IsBindless) {
        glBindTextureUnit(unit, 0);
    }
}
