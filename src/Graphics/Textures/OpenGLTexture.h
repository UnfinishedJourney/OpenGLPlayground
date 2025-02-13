#pragma once
#include "ITexture.h"
#include "TextureData.h"
#include "TextureConfig.h"
#include <glad/glad.h>
#include <cstdint>

namespace graphics {

    /**
     * @brief Represents a standard 2D OpenGL texture.
     *
     * Supports LDR or HDR data, optional sRGB internal formats,
     * mipmaps, anisotropy, and optionally bindless textures.
     */
    class OpenGLTexture : public ITexture {
    public:
        /**
         * @brief Creates a texture from the provided image data and configuration.
         * @param data   The image data.
         * @param config Configuration parameters.
         * @throws std::runtime_error if texture creation fails.
         */
        OpenGLTexture(const TextureData& data, const TextureConfig& config);
        ~OpenGLTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth() const override { return width_; }
        uint32_t GetHeight() const override { return height_; }
        uint64_t GetBindlessHandle() const override { return bindless_handle_; }
        bool IsBindless() const override { return is_bindless_; }

        /// Returns the raw OpenGL texture ID.
        [[nodiscard]] GLuint GetTextureID() const { return texture_id_; }

    private:
        GLuint texture_id_ = 0;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        uint64_t bindless_handle_ = 0;
        bool is_bindless_ = false;

        /// Determines the final internal format based on config and channel count.
        static GLenum ResolveInternalFormat(const TextureConfig& config, int channels);
    };

} // namespace graphics
