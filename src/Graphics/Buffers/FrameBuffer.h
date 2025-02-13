#pragma once

#include <glad/glad.h>
#include <vector>
#include <stdexcept>

namespace graphics {

    /**
     * @brief Describes a texture attachment for a framebuffer.
     */
    struct FrameBufferTextureAttachment {
        GLenum attachment_type;  ///< e.g., GL_COLOR_ATTACHMENT0
        GLenum internal_format;  ///< e.g., GL_RGBA8
        GLenum format;           ///< e.g., GL_RGBA
        GLenum type;             ///< e.g., GL_UNSIGNED_BYTE
    };

    /**
     * @brief Encapsulates an OpenGL framebuffer along with its attachments.
     *
     * Manages color and depth attachments (optionally multisampled) using RAII.
     */
    class FrameBuffer {
    public:
        /**
         * @brief Constructs a framebuffer with given dimensions and attachments.
         *
         * @param width        Framebuffer width.
         * @param height       Framebuffer height.
         * @param attachments  List of color attachment specifications.
         * @param has_depth    Whether to include a depth texture.
         * @param samples      Number of samples (multisampling if > 1).
         *
         * @throws std::runtime_error if framebuffer creation fails.
         */
        FrameBuffer(int width,
            int height,
            const std::vector<FrameBufferTextureAttachment>& attachments,
            bool has_depth = true,
            int samples = 1);

        ~FrameBuffer();

        // Non-copyable
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        // Movable
        FrameBuffer(FrameBuffer&& other) noexcept;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

        /// Binds the framebuffer for rendering.
        void Bind() const;

        /// Unbinds (binds the default framebuffer).
        void Unbind() const;

        /**
         * @brief Returns the texture ID for a given color attachment.
         * @param attachment Attachment enum (e.g., GL_COLOR_ATTACHMENT0 + n).
         * @return Texture ID if found, or 0.
         */
        [[nodiscard]] GLuint GetTexture(GLenum attachment) const;

        /// Returns the depth texture ID.
        [[nodiscard]] GLuint GetDepthTexture() const;

        /// Resizes the framebuffer and reinitializes attachments.
        void Resize(int new_width, int new_height);

        /**
         * @brief Blits (or resolves) from this framebuffer to a target framebuffer.
         *
         * @param target_fbo The destination framebuffer.
         * @param mask       Which buffers to blit (e.g., GL_COLOR_BUFFER_BIT).
         * @param filter     Filtering mode (GL_NEAREST or GL_LINEAR).
         */
        void BlitTo(const FrameBuffer& target_fbo,
            GLbitfield mask = GL_COLOR_BUFFER_BIT,
            GLenum filter = GL_LINEAR) const;

        [[nodiscard]] GLuint GetRendererID() const { return renderer_id_; }
        [[nodiscard]] int GetWidth()  const { return width_; }
        [[nodiscard]] int GetHeight() const { return height_; }
        [[nodiscard]] int GetSamples() const { return samples_; }
        [[nodiscard]] bool HasDepth() const { return has_depth_; }

    private:
        // Helper functions for texture attachment creation.
        GLuint CreateColorAttachment(const FrameBufferTextureAttachment& att) const;
        GLuint CreateDepthAttachment() const;

        /// Initializes (or reinitializes) the framebuffer.
        void Initialize();

        /// Releases OpenGL resources.
        void Cleanup();

        GLuint renderer_id_{ 0 };               ///< OpenGL framebuffer handle.
        std::vector<GLuint> color_textures_;      ///< Color texture attachments.
        GLuint depth_texture_{ 0 };              ///< Depth texture attachment (if any).

        std::vector<FrameBufferTextureAttachment> attachments_;
        int width_{ 0 };
        int height_{ 0 };
        bool has_depth_{ false };
        int samples_{ 1 };
    };

} // namespace graphics
