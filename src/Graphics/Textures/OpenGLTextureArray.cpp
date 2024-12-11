#include "OpenGLTextureArray.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <cmath>

OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& filePaths, const TextureConfig& config) {
    if (filePaths.empty()) throw std::runtime_error("No files for texture array.");

    std::vector<TextureData> textures(filePaths.size());
    for (size_t i = 0; i < filePaths.size(); i++) {
        if (!textures[i].LoadFromFile(filePaths[i]))
            throw std::runtime_error("Failed to load: " + filePaths[i]);
    }

    m_Width = textures[0].GetWidth();
    m_Height = textures[0].GetHeight();
    for (auto& t : textures) {
        if ((uint32_t)t.GetWidth() != m_Width || (uint32_t)t.GetHeight() != m_Height)
            throw std::runtime_error("All textures in array must have same dimensions.");
    }

    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_TextureID);
    if (!m_TextureID) throw std::runtime_error("Failed to create texture array.");

    int levels = config.generateMips ? (int)std::floor(std::log2(std::max(m_Width, m_Height))) + 1 : 1;
    glTextureStorage3D(m_TextureID, levels, config.internalFormat, m_Width, m_Height, (GLsizei)textures.size());

    for (size_t i = 0; i < textures.size(); i++) {
        glTextureSubImage3D(m_TextureID, 0, 0, 0, (GLint)i, m_Width, m_Height, 1,
            GL_RGBA, GL_UNSIGNED_BYTE, textures[i].GetData());
    }

    if (config.generateMips) glGenerateTextureMipmap(m_TextureID);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT);
    glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR);

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

OpenGLTextureArray::~OpenGLTextureArray() {
    if (m_IsBindless && m_BindlessHandle) glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
}

void OpenGLTextureArray::Bind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, m_TextureID);
}

void OpenGLTextureArray::Unbind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, 0);
}