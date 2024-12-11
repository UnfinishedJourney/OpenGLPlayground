#include "OpenGLTexture.h"
#include "Utilities/Logger.h"
#include <cmath>

OpenGLTexture::OpenGLTexture(const TextureData& data, const TextureConfig& config) {
    m_Width = data.GetWidth();
    m_Height = data.GetHeight();

    glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
    if (!m_TextureID) throw std::runtime_error("Failed to create texture.");

    int levels = config.generateMips ? (int)std::floor(std::log2(std::max(m_Width, m_Height))) + 1 : 1;
    glTextureStorage2D(m_TextureID, levels, config.internalFormat, m_Width, m_Height);
    glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, data.GetData());

    if (config.generateMips) glGenerateTextureMipmap(m_TextureID);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT);

    if (config.useAnisotropy && GLAD_GL_EXT_texture_filter_anisotropic) {
        GLfloat maxAniso;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    }

    if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
        m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
        if (m_BindlessHandle) {
            glMakeTextureHandleResidentARB(m_BindlessHandle);
            m_IsBindless = true;
        }
    }
}

OpenGLTexture::~OpenGLTexture() {
    if (m_IsBindless && m_BindlessHandle) glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
}

void OpenGLTexture::Bind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, m_TextureID);
}

void OpenGLTexture::Unbind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, 0);
}