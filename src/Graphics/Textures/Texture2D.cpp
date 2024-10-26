#include "Graphics/Textures/Texture2D.h"
#include <stb_image.h>
#include "Utilities/Logger.h"

Texture2D::Texture2D(std::filesystem::path filePath)
    : TextureBase(std::move(filePath)) {
    LoadTexture();
}

void Texture2D::LoadTexture() {
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(m_FilePath.string().c_str(), &width, &height, &channels, 4);
    if (!data) {
        Logger::GetLogger()->error("Failed to load texture: '{}'", m_FilePath.string());
        throw std::runtime_error("Failed to load texture: " + m_FilePath.string());
    }

    m_Width = width;
    m_Height = height;
    m_BPP = channels * 8;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    if (m_RendererID == 0) {
        stbi_image_free(data);
        Logger::GetLogger()->error("Failed to create OpenGL texture.");
        throw std::runtime_error("Failed to create OpenGL texture.");
    }

    glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Height);
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateTextureMipmap(m_RendererID);

    stbi_image_free(data);

    Logger::GetLogger()->info("Loaded Texture2D from '{}', Renderer ID: {}", m_FilePath.string(), m_RendererID);
}

void Texture2D::Bind(GLuint slot) const {
    glBindTextureUnit(slot, m_RendererID);
    Logger::GetLogger()->debug("Bound Texture2D ID {} to slot {}", m_RendererID, slot);
}

void Texture2D::Unbind(GLuint slot) const {
    glBindTextureUnit(slot, 0);
    Logger::GetLogger()->debug("Unbound Texture2D from slot {}", slot);
}