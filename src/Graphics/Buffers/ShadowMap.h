#pragma once

#include <glad/glad.h>
#include <stdexcept>

namespace graphics {

    /**
     * @brief Manages a shadow map by creating a depth texture and an associated FBO.
     *
     * This class encapsulates the OpenGL calls needed to create a depth texture for
     * shadow mapping and the framebuffer (FBO) used to render depth values.
     */
    class ShadowMap {
    public:
        /**
         * @brief Constructs a ShadowMap with the given resolution.
         * @param width  The texture width.
         * @param height The texture height.
         * @throws std::runtime_error if the framebuffer is incomplete.
         */
        ShadowMap(GLsizei width, GLsizei height);

        ~ShadowMap();

        // Non-copyable
        ShadowMap(const ShadowMap&) = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        // Movable
        ShadowMap(ShadowMap&& other) noexcept;
        ShadowMap& operator=(ShadowMap&& other) noexcept;

        /// Binds the shadow map FBO for writing depth values.
        void BindForWriting() const;

        /// Unbinds the FBO (binds the default framebuffer).
        void Unbind() const;

        /// Returns the depth texture ID.
        [[nodiscard]] GLuint GetDepthTexture() const { return depthTexture_; }

        [[nodiscard]] GLsizei GetWidth() const { return width_; }
        [[nodiscard]] GLsizei GetHeight() const { return height_; }

    private:
        /// Releases all OpenGL resources.
        void Cleanup();

        GLuint rendererId_{ 0 };    ///< The framebuffer (FBO) handle.
        GLuint depthTexture_{ 0 };  ///< The depth texture attached to the FBO.
        GLsizei width_{ 0 };         ///< Width of the shadow map.
        GLsizei height_{ 0 };        ///< Height of the shadow map.
    };

} // namespace graphics