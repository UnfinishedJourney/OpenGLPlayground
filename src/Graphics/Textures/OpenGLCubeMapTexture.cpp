#include "OpenGLCubeMapTexture.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <cmath>

OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces, const TextureConfig& config) {
    std::array<TextureData, 6> data;
    for (size_t i = 0; i < 6; i++) {
        if (!data[i].LoadFromFile(faces[i].string(), false))
            throw std::runtime_error("Failed to load cube face: " + faces[i].string());
    }

    m_Width = data[0].GetWidth();
    m_Height = data[0].GetHeight();
    for (auto& d : data) {
        if ((uint32_t)d.GetWidth() != m_Width || (uint32_t)d.GetHeight() != m_Height)
            throw std::runtime_error("All cube faces must have same dimensions.");
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
    if (!m_TextureID) throw std::runtime_error("Failed to create cube map.");

    int levels = config.generateMips ? (int)std::floor(std::log2(std::max(m_Width, m_Height))) + 1 : 1;
    glTextureStorage2D(m_TextureID, levels, config.internalFormat, m_Width, m_Height);

    for (size_t i = 0; i < 6; i++) {
        glTextureSubImage3D(m_TextureID, 0, 0, 0, (GLint)i, m_Width, m_Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data[i].GetData());
    }

    if (config.generateMips) glGenerateTextureMipmap(m_TextureID);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR);

    if (config.useAnisotropy) {
        GLfloat maxAniso;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
    }

    if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
        m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
        if (m_BindlessHandle) {
            glMakeTextureHandleResidentARB(m_BindlessHandle);
            m_IsBindless = true;
        }
    }
}

OpenGLCubeMapTexture::~OpenGLCubeMapTexture() {
    if (m_IsBindless && m_BindlessHandle) glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
}

void OpenGLCubeMapTexture::Bind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, m_TextureID);
}

void OpenGLCubeMapTexture::Unbind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, 0);
}