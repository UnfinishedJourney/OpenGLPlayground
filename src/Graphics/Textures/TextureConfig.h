#pragma once
#include <glad/glad.h>

namespace graphics {

    /**
     * @brief Configuration parameters for texture creation.
     *
     * Includes internal format, wrapping, filtering, mipmap generation,
     * anisotropy, bindless usage, and flags for HDR and sRGB.
     */
    struct TextureConfig {
        GLenum internal_format_ = GL_RGBA8;
        GLenum wrap_s_ = GL_REPEAT;
        GLenum wrap_t_ = GL_REPEAT;
        GLenum wrap_r_ = GL_REPEAT;  // For cube maps or 3D textures.
        GLenum min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
        GLenum mag_filter_ = GL_LINEAR;
        bool generate_mips_ = true;
        bool use_anisotropy_ = true;
        bool use_bindless_ = false;
        bool is_hdr_ = false;   ///< Expect float data (e.g. stbi_loadf).
        bool is_srgb_ = false;  ///< Use an sRGB internal format if LDR.
    };

} // namespace graphics