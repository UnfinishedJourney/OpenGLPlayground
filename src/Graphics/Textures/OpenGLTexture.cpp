#include "OpenGLTexture.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // Contains GLCall macro.
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace graphics {

    static inline GLenum DataFormatForHDR(bool is_hdr) {
        return is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    }

    GLenum OpenGLTexture::ResolveInternalFormat(const TextureConfig& config, int channels) {
        GLenum chosen = config.internal_format_;
        if (config.is_hdr_) {
            // Switch to a higher-precision format if HDR is requested.
            chosen = (chosen == GL_RGBA8 || chosen == GL_SRGB8_ALPHA8) ? GL_RGBA16F : chosen;
        }
        else if (config.is_srgb_) {
            if (chosen == GL_RGBA8)
                chosen = GL_SRGB8_ALPHA8;
        }
        return chosen;
    }

    OpenGLTexture::OpenGLTexture(const TextureData& data, const TextureConfig& config) {
        TextureConfig cfg = config; // local copy for potential adjustments
        cfg.internal_format_ = ResolveInternalFormat(cfg, data.GetChannels());

        width_ = static_cast<uint32_t>(data.GetWidth());
        height_ = static_cast<uint32_t>(data.GetHeight());

        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &texture_id_));
        if (texture_id_ == 0) {
            throw std::runtime_error("OpenGLTexture: Failed to create texture.");
        }

        int levels = cfg.generate_mips_
            ? static_cast<int>(std::floor(std::log2(std::max(width_, height_)))) + 1
            : 1;

        GLCall(glTextureStorage2D(texture_id_, levels, cfg.internal_format_, width_, height_));

        GLenum data_type = DataFormatForHDR(data.IsHDR());
        GLenum upload_format = GL_RGBA;  // Assume data is RGBA.
        const void* pixels = data.IsHDR()
            ? static_cast<const void*>(data.GetDataFloat())
            : static_cast<const void*>(data.GetDataU8());
        if (pixels) {
            GLCall(glTextureSubImage2D(texture_id_, 0, 0, 0, width_, height_, upload_format, data_type, pixels));
        }
        if (cfg.generate_mips_) {
            GLCall(glGenerateTextureMipmap(texture_id_));
        }
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MIN_FILTER, cfg.min_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MAG_FILTER, cfg.mag_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_S, cfg.wrap_s_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_T, cfg.wrap_t_));

        if (cfg.use_anisotropy_) {
            GLfloat max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
            GLCall(glTextureParameterf(texture_id_, GL_TEXTURE_MAX_ANISOTROPY, max_aniso));
        }

        if (cfg.use_bindless_ && GLAD_GL_ARB_bindless_texture) {
            bindless_handle_ = glGetTextureHandleARB(texture_id_);
            if (bindless_handle_) {
                glMakeTextureHandleResidentARB(bindless_handle_);
                is_bindless_ = true;
            }
        }

        Logger::GetLogger()->info("OpenGLTexture: Created texture (ID={}) {}x{}; HDR={}, sRGB={}.",
            texture_id_, width_, height_, cfg.is_hdr_, cfg.is_srgb_);
    }

    OpenGLTexture::~OpenGLTexture() {
        if (is_bindless_ && bindless_handle_) {
            glMakeTextureHandleNonResidentARB(bindless_handle_);
        }
        if (texture_id_) {
            GLCall(glDeleteTextures(1, &texture_id_));
        }
    }

    void OpenGLTexture::Bind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, texture_id_));
        }
    }

    void OpenGLTexture::Unbind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace graphics