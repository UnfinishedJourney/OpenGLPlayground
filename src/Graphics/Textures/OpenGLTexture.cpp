#include "OpenGLTexture.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // Contains GLCall macro
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace Graphics {

    static inline GLenum DataFormatForHDR(bool isHDR) {
        return isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
    }

    GLenum OpenGLTexture::ResolveInternalFormat(const TextureConfig& config, int channels) {
        GLenum chosen = config.internalFormat;
        if (config.isHDR) {
            // If the users format was GL_RGBA8 or GL_SRGB8_ALPHA8 but HDR is requested, switch to a floating-point format.
            chosen = (chosen == GL_RGBA8 || chosen == GL_SRGB8_ALPHA8)
                ? GL_RGBA16F
                : chosen;
        }
        else if (config.isSRGB) {
            if (chosen == GL_RGBA8) {
                chosen = GL_SRGB8_ALPHA8;
            }
        }
        return chosen;
    }

    OpenGLTexture::OpenGLTexture(const TextureData& data, const TextureConfig& config) {
        TextureConfig cfg = config; // local copy for potential adjustments
        cfg.internalFormat = ResolveInternalFormat(cfg, data.GetChannels());

        m_Width = static_cast<uint32_t>(data.GetWidth());
        m_Height = static_cast<uint32_t>(data.GetHeight());

        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID));
        if (!m_TextureID) {
            throw std::runtime_error("Failed to create GL texture object.");
        }

        int levels = cfg.generateMips
            ? static_cast<int>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1
            : 1;

        GLCall(glTextureStorage2D(m_TextureID, levels, cfg.internalFormat, m_Width, m_Height));

        GLenum dataType = DataFormatForHDR(data.IsHDR());
        // Here we assume the data has been forced to 4 channels (GL_RGBA).
        GLenum uploadFormat = GL_RGBA;

        const void* pixels = data.IsHDR()
            ? static_cast<const void*>(data.GetDataFloat())
            : static_cast<const void*>(data.GetDataU8());
        if (pixels) {
            GLCall(glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, uploadFormat, dataType, pixels));
        }
        if (cfg.generateMips) {
            GLCall(glGenerateTextureMipmap(m_TextureID));
        }
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, cfg.minFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, cfg.magFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, cfg.wrapS));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, cfg.wrapT));

        if (cfg.useAnisotropy) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            GLCall(glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso));
        }

        if (cfg.useBindless && GLAD_GL_ARB_bindless_texture) {
            m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
            if (m_BindlessHandle) {
                glMakeTextureHandleResidentARB(m_BindlessHandle);
                m_IsBindless = true;
            }
        }

        Logger::GetLogger()->info("Created OpenGLTexture (ID={}) {}x{}; HDR={}, sRGB={}.",
            m_TextureID, m_Width, m_Height, cfg.isHDR, cfg.isSRGB);
    }

    OpenGLTexture::~OpenGLTexture() {
        if (m_IsBindless && m_BindlessHandle) {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
        }
        if (m_TextureID) {
            GLCall(glDeleteTextures(1, &m_TextureID));
        }
    }

    void OpenGLTexture::Bind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, m_TextureID));
        }
    }

    void OpenGLTexture::Unbind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace Graphics