#pragma once
#include "ITexture.h"
#include "TextureData.h"
#include "TextureConfig.h"
#include <glad/glad.h>
#include <cstdint>

namespace Graphics {

    /**
     * @brief Represents a standard 2D OpenGL texture.
     *
     * It supports LDR or HDR data, optional sRGB internal formats,
     * mipmaps, anisotropy, and optionally bindless textures.
     */
    class OpenGLTexture : public ITexture {
    public:
        /**
         * @brief Creates a texture from the provided image data and configuration.
         */
        OpenGLTexture(const TextureData& data, const TextureConfig& config);
        ~OpenGLTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth()  const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
        bool     IsBindless()        const override { return m_IsBindless; }

        /// Optionally get the raw OpenGL texture ID.
        [[nodiscard]] GLuint GetTextureID() const { return m_TextureID; }

    private:
        GLuint m_TextureID = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint64_t m_BindlessHandle = 0;
        bool m_IsBindless = false;

        /// Determines the final internal format based on config and channel count.
        static GLenum ResolveInternalFormat(const TextureConfig& config, int channels);
    };

} // namespace Graphics
