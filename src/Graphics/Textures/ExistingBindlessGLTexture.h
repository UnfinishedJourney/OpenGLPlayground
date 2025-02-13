#pragma once
#include "ITexture.h"
#include <glad/glad.h>
#include <cstdint>

namespace graphics {

    /**
     * @brief Wraps an externally created GL texture so that it can be made bindless.
     */
    class ExistingBindlessGLTexture : public ITexture {
    public:
        /**
         * @brief Wraps an existing GL texture.
         * @param id The GL texture ID.
         * @param w Width of the texture.
         * @param h Height of the texture.
         * @param makeBindless If true, obtains a bindless handle.
         */
        ExistingBindlessGLTexture(GLuint id, int w, int h, bool makeBindless);
        ~ExistingBindlessGLTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth() const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
        bool IsBindless() const override { return m_IsBindless; }

    private:
        GLuint m_TextureID = 0;
        int m_Width = 0;
        int m_Height = 0;
        bool m_IsBindless = false;
        uint64_t m_BindlessHandle = 0;
    };

} // namespace graphics