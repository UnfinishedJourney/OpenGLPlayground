#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "BufferDeleter.h"

/**
 * @brief Describes a texture attachment for a FrameBuffer.
 */
struct FrameBufferTextureAttachment {
    GLenum attachmentType; // e.g., GL_COLOR_ATTACHMENT0
    GLenum internalFormat; // e.g., GL_RGBA8
    GLenum format;         // e.g., GL_RGBA
    GLenum type;           // e.g., GL_UNSIGNED_BYTE
};

/**
 * @brief A class encapsulating an OpenGL Framebuffer.
 *        Supports both single-sample and multisample attachments.
 */
class FrameBuffer {
public:
    /**
     * @param width          The width of the framebuffer attachments.
     * @param height         The height of the framebuffer attachments.
     * @param attachments    A list of color (or other) attachments to create.
     * @param hasDepth       Whether to include a depth attachment.
     * @param samples        If > 1, creates a multisample framebuffer.
     */
    FrameBuffer(int width,
        int height,
        const std::vector<FrameBufferTextureAttachment>& attachments,
        bool hasDepth = true,
        int samples = 1);

    ~FrameBuffer();

    // Non-copyable, move-only
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer&&) noexcept = default;
    FrameBuffer& operator=(FrameBuffer&&) noexcept = default;

    // Bind/unbind
    void Bind() const;
    void Unbind() const;

    /**
     * @brief Returns the texture ID for a specific attachment.
     *        Only valid for single-sample attachments. For multisample, see note below.
     */
    GLuint GetTexture(GLenum attachment) const;

    /**
     * @brief Resizes the FBO attachments to the new width/height.
     */
    void Resize(int newWidth, int newHeight);

    /**
     * @brief Blit/resolve from this FBO to a target FBO (e.g. single-sample).
     *        Used to resolve MSAA color into a non-multisample texture if needed.
     * @param targetFBO   The destination FBO (e.g., a single-sample FBO).
     * @param mask        Which buffers to blit (GL_COLOR_BUFFER_BIT, etc.).
     * @param filter      Typically GL_NEAREST or GL_LINEAR for color.
     */
    void BlitTo(FrameBuffer& targetFBO,
        GLbitfield mask = GL_COLOR_BUFFER_BIT,
        GLenum filter = GL_LINEAR) const;

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }
    [[nodiscard]] int GetWidth()      const { return m_Width; }
    [[nodiscard]] int GetHeight()     const { return m_Height; }
    [[nodiscard]] int GetSamples()    const { return m_Samples; }
    [[nodiscard]] bool HasDepth()     const { return m_HasDepth; }

private:
    void Initialize(int width,
        int height,
        const std::vector<FrameBufferTextureAttachment>& attachments,
        bool hasDepth,
        int samples);
    void Cleanup();

    std::unique_ptr<GLuint, FrameBufferDeleter>    m_RendererIDPtr;
    std::vector<std::unique_ptr<GLuint, TextureDeleter>> m_Textures; // For color attachments
    std::unique_ptr<GLuint, RenderBufferDeleter>   m_DepthRenderBufferPtr;

    // Store attachments for resizing
    std::vector<FrameBufferTextureAttachment> m_Attachments;

    int  m_Width = 0;
    int  m_Height = 0;
    bool m_HasDepth = false;
    int  m_Samples = 1; // 1 means single-sample (no MSAA)
};
