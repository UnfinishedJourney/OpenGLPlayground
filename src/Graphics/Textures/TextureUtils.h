#pragma once
#include "TextureConfig.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>

namespace graphics {

    /**
     * @brief Determine final internal format given config (HDR, sRGB, etc.) and channel count.
     */
    inline GLenum ResolveInternalFormat(const TextureConfig& config, int channels)
    {
        // Start with the config's format:
        GLenum chosen = config.internal_format_;

        // If HDR requested, pick a float variant if needed
        if (config.is_hdr_) {
            if (chosen == GL_RGBA8 || chosen == GL_SRGB8_ALPHA8) {
                chosen = GL_RGBA16F; // or GL_RGBA32F
            }
        }
        else if (config.is_srgb_) {
            if (chosen == GL_RGBA8) {
                chosen = GL_SRGB8_ALPHA8;
            }
            // Could add logic if channels=3 => GL_SRGB8, etc.
        }

        return chosen;
    }

    /**
     * @brief Decide data type for glTexSubImage calls if the image is HDR (float) or not.
     */
    inline GLenum DataTypeForHDR(bool is_hdr)
    {
        return is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    }

    /**
     * @brief Helper to set GL texture parameters for wrap, filter, mipmaps, anisotropy, etc.
     */
    inline void SetupTextureParameters(GLuint texID, const TextureConfig& cfg, bool isCubeMap = false)
    {
        if (cfg.generate_mips_) {
            glGenerateTextureMipmap(texID);
        }

        glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, cfg.min_filter_);
        glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, cfg.mag_filter_);

        glTextureParameteri(texID, GL_TEXTURE_WRAP_S, cfg.wrap_s_);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_T, cfg.wrap_t_);
        if (isCubeMap) {
            glTextureParameteri(texID, GL_TEXTURE_WRAP_R, cfg.wrap_r_);
        }

        if (cfg.use_anisotropy_) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            glTextureParameterf(texID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
        }
    }

} // namespace graphics