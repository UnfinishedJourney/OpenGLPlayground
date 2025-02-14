#pragma once
#include "GLBaseTexture.h"
#include "Bitmap.h"
#include "TextureConfig.h"

namespace graphics {

    class OpenGLTexture : public GLBaseTexture {
    public:
        OpenGLTexture(const Bitmap& image, const TextureConfig& config);
        ~OpenGLTexture() override = default;
        GLuint GetTextureID() const { return texture_id_; }
    };

} // namespace graphics