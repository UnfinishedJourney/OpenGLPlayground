#pragma once
#include "GLBaseTexture.h"
#include "Bitmap.h"
#include "TextureConfig.h"

namespace graphics {

    /**
     * @brief Standard 2D OpenGL texture. Inherits from GLBaseTexture.
     */
    class OpenGLTexture : public GLBaseTexture
    {
    public:
        /**
         * @brief Construct a 2D texture from the given Bitmap + config.
         */
        OpenGLTexture(const Bitmap& image, const TextureConfig& config);

        ~OpenGLTexture() override = default;

        /// If needed, expose the raw GL texture ID
        GLuint GetTextureID() const { return texture_id_; }

    private:
        // We might add some private helper or put it in a separate "TextureHelpers.h"
    };

} // namespace graphics