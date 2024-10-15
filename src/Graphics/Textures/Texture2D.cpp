#include "Graphics/Textures/Texture2D.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

Texture2D::Texture2D()
    : TextureBase()
{
    Logger::GetLogger()->info("Initialized empty Texture2D.");
}

Texture2D::Texture2D(const std::string& path)
    : TextureBase(path)
{
    Logger::GetLogger()->info("Initialized Texture2D from path: {}.", path);
    LoadTexture(path);
}

void Texture2D::LoadTexture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);

    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    if (!m_LocalBuffer)
    {
        Logger::GetLogger()->error("Failed to load texture from path: {}.", path);
        throw std::runtime_error("Failed to load texture: " + path);
    }
    Logger::GetLogger()->info("Loaded texture from path: {}. Dimensions: {}x{}, Channels: {}.",
        path, m_Width, m_Height, m_BPP);

    GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID));
    if (m_RendererID == 0)
    {
        stbi_image_free(m_LocalBuffer);
        Logger::GetLogger()->error("Failed to create OpenGL texture.");
        throw std::runtime_error("Failed to create OpenGL texture.");
    }
    Logger::GetLogger()->info("Created OpenGL texture with ID {}.", m_RendererID);

    GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT));

    GLCall(glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Height));

    GLCall(glTextureSubImage2D(
        m_RendererID,
        0, // mipmap level
        0, 0, // x and y offsets
        m_Width, m_Height, // width and height
        GL_RGBA, // format
        GL_UNSIGNED_BYTE, // type
        m_LocalBuffer // data
    ));

    Logger::GetLogger()->info("Uploaded data to texture ID {}.", m_RendererID);

    GLCall(glGenerateTextureMipmap(m_RendererID));
    Logger::GetLogger()->info("Generated mipmaps for texture ID {}.", m_RendererID);

    stbi_image_free(m_LocalBuffer);
    m_LocalBuffer = nullptr;
    Logger::GetLogger()->info("Freed image memory for texture ID {}.", m_RendererID);
}

void Texture2D::Bind(unsigned int slot) const
{
    GLCall(glBindTextureUnit(slot, m_RendererID));
    Logger::GetLogger()->debug("Bound Texture2D ID {} to slot {}.", m_RendererID, slot);
}

void Texture2D::Unbind(unsigned int slot) const
{
    GLCall(glBindTextureUnit(slot, 0));
    Logger::GetLogger()->debug("Unbound Texture2D from slot {}.", slot);
}