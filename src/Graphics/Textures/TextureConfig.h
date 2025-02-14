#pragma once
#include <glad/glad.h>

namespace graphics {

    struct TextureConfig {
        GLenum internal_format_ = GL_RGBA8;
        GLenum wrap_s_ = GL_REPEAT;
        GLenum wrap_t_ = GL_REPEAT;
        GLenum wrap_r_ = GL_REPEAT;
        GLenum min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
        GLenum mag_filter_ = GL_LINEAR;
        bool generate_mips_ = true;
        bool use_anisotropy_ = true;
        bool use_bindless_ = false;
        bool is_hdr_ = false;
        bool is_srgb_ = false;
    };

} // namespace graphics