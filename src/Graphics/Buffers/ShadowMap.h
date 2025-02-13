#pragma once

#include <glad/glad.h>

namespace graphics {

    /**
     * @brief Manages a shadow map by creating a depth texture and associated FBO.
     */
    class ShadowMap {
    public:
        /**
         * @brief Constructs a shadow map with the given resolution.
         * @param width Texture width.
         * @param height Texture height.
         */
        ShadowMap(GLsizei width, GLsizei height);

        ~ShadowMap();

        /**
         * @brief Binds the shadow map FBO for writing depth values.
         */
        void BindForWriting() const;

        /**
         * @brief Unbinds the FBO.
         */
        void Unbind() const;

        /**
         * @brief Returns the depth texture ID.
         */
        GLuint GetDepthTexture() const { return m_DepthTex; }

        GLsizei GetWidth() const { return m_Width; }
        GLsizei GetHeight() const { return m_Height; }

    private:
        GLuint m_ShadowFBO = 0;
        GLuint m_DepthTex = 0;
        GLsizei m_Width, m_Height;
    };

} // namespace graphics