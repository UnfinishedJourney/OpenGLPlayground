#include "OpenGLTexture.h"
#include <cmath>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // for GLCall macros

static inline GLenum DataFormatForHDR(bool isHDR) {
    return isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
}

GLenum OpenGLTexture::ResolveInternalFormat(const TextureConfig& config, int channels)
{
    // If the user manually set config.internalFormat, we honor that unless
    // they set isHDR/isSRGB, in which case we might override.
    // This is flexible. If you prefer strict override, comment it out.
    GLenum chosen = config.internalFormat;

    // If HDR
    if (config.isHDR) {
        // Use RGBA16F by default (some users prefer RGBA32F).
        chosen = (chosen == GL_RGBA8 || chosen == GL_SRGB8_ALPHA8)
            ? GL_RGBA16F
            : chosen;
    }
    else if (config.isSRGB) {
        // Use sRGB8_ALPHA8 if user hasn't set something else
        if (chosen == GL_RGBA8) {
            chosen = GL_SRGB8_ALPHA8;
        }
    }

    // In case the user has a grayscale or 3-channel image, you can adapt
    // but typically we do RGBA. This is optional logic:
    // e.g. if (channels == 3 && !config.isHDR) chosen = GL_SRGB8; // etc.

    return chosen;
}

OpenGLTexture::OpenGLTexture(const TextureData& data, TextureConfig config)
{
    // Determine final internal format from config & data
    config.internalFormat = ResolveInternalFormat(config, data.GetChannels());

    m_Width = static_cast<uint32_t>(data.GetWidth());
    m_Height = static_cast<uint32_t>(data.GetHeight());

    // 1) Create a GL texture
    GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID));
    if (!m_TextureID) {
        throw std::runtime_error("Failed to create GL texture object.");
    }

    // 2) Compute the number of mip levels
    int levels = config.generateMips
        ? static_cast<int>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1
        : 1;

    // 3) Allocate storage
    GLCall(glTextureStorage2D(m_TextureID, levels, config.internalFormat, m_Width, m_Height));

    // 4) Upload the data
    GLenum dataType = DataFormatForHDR(data.IsHDR());
    GLenum uploadFormat = GL_RGBA; // Assuming we forced RGBA in TextureData
    // If you prefer to handle real channel count, do so here:
    //  if (data.GetChannels() == 3) uploadFormat = GL_RGB;
    //  if (data.GetChannels() == 1) uploadFormat = GL_RED; etc.

    const void* pixels = data.IsHDR()
        ? static_cast<const void*>(data.GetDataFloat())
        : static_cast<const void*>(data.GetDataU8());

    if (pixels) {
        GLCall(glTextureSubImage2D(
            m_TextureID, 0, 0, 0,
            m_Width, m_Height,
            uploadFormat,
            dataType,
            pixels
        ));
    }

    // 5) Mipmaps
    if (config.generateMips) {
        GLCall(glGenerateTextureMipmap(m_TextureID));
    }

    // 6) Set filters/wrap
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));

    // 7) Optional anisotropy
    if (config.useAnisotropy) {
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        GLCall(glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso));
    }

    // 8) Optional bindless
    if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
        m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
        if (m_BindlessHandle) {
            glMakeTextureHandleResidentARB(m_BindlessHandle);
            m_IsBindless = true;
        }
    }

    Logger::GetLogger()->info("Created OpenGLTexture (ID={}) {}x{}; HDR={}, sRGB={}.",
        m_TextureID, m_Width, m_Height, config.isHDR, config.isSRGB);
}

OpenGLTexture::~OpenGLTexture()
{
    // Clean up bindless if used
    if (m_IsBindless && m_BindlessHandle) {
        glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    }
    // Delete texture
    if (m_TextureID) {
        GLCall(glDeleteTextures(1, &m_TextureID));
    }
}

void OpenGLTexture::Bind(uint32_t unit) const
{
    if (!m_IsBindless) {
        GLCall(glBindTextureUnit(unit, m_TextureID));
    }
}

void OpenGLTexture::Unbind(uint32_t unit) const
{
    if (!m_IsBindless) {
        GLCall(glBindTextureUnit(unit, 0));
    }
}