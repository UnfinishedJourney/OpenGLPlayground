#pragma once

#include <glad/glad.h>
#include <vector>
#include <stdexcept>

namespace graphics {

    /**
     * @brief Specifies a texture attachment for a framebuffer.
     */
    struct FrameBufferTextureAttachment {
        GLenum attachment_type;  ///< e.g. GL_COLOR_ATTACHMENT0
        GLenum internal_format;  ///< e.g. GL_RGBA8
        GLenum format;           ///< e.g. GL_RGBA
        GLenum type;             ///< e.g. GL_UNSIGNED_BYTE
    };

    /**
     * @brief Encapsulates an OpenGL framebuffer and its attachments.
     *
     * Manages color and depth attachments (optionally multisampled) using RAII.
     */
    class FrameBuffer {
    public:
        FrameBuffer(int width,
            int height,
            const std::vector<FrameBufferTextureAttachment>& attachments,
            bool hasDepth = true,
            int samples = 1);
        ~FrameBuffer();

        // Non-copyable.
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;
        // Movable.
        FrameBuffer(FrameBuffer&& other) noexcept;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

        void Bind() const;
        void Unbind() const;

        [[nodiscard]] GLuint GetTexture(GLenum attachment) const;
        [[nodiscard]] GLuint GetDepthTexture() const;
        void Resize(int newWidth, int newHeight);
        void BlitTo(const FrameBuffer& targetFbo,
            GLbitfield mask = GL_COLOR_BUFFER_BIT,
            GLenum filter = GL_LINEAR) const;

        [[nodiscard]] GLuint GetRendererID() const { return rendererId_; }
        [[nodiscard]] int GetWidth()  const { return width_; }
        [[nodiscard]] int GetHeight() const { return height_; }
        [[nodiscard]] int GetSamples() const { return samples_; }
        [[nodiscard]] bool HasDepth() const { return hasDepth_; }

    private:
        GLuint CreateColorAttachment(const FrameBufferTextureAttachment& att) const;
        GLuint CreateDepthAttachment() const;
        void Initialize();
        void Cleanup();

        GLuint rendererId_{ 0 };
        std::vector<GLuint> colorTextures_;
        GLuint depthTexture_{ 0 };

        std::vector<FrameBufferTextureAttachment> attachments_;
        int width_{ 0 };
        int height_{ 0 };
        bool hasDepth_{ false };
        int samples_{ 1 };
    };

} // namespace graphics