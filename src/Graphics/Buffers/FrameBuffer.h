#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "BufferDeleter.h"

struct FrameBufferTextureAttachment {
    GLenum attachmentType; // e.g., GL_COLOR_ATTACHMENT0
    GLenum internalFormat; // e.g., GL_RGBA8
    GLenum format;         // e.g., GL_RGBA
    GLenum type;           // e.g., GL_UNSIGNED_BYTE
};

class FrameBuffer {
public:
    FrameBuffer(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth = true);
    ~FrameBuffer() = default;

    FrameBuffer(FrameBuffer&&) noexcept = default;
    FrameBuffer& operator=(FrameBuffer&&) noexcept = default;

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    // Get the texture ID for a specific attachment
    GLuint GetTexture(GLenum attachment) const;

    // Resize the framebuffer and its attachments
    void Resize(int newWidth, int newHeight);

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }
    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }

private:
    void Initialize(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth);
    void Cleanup();

    std::unique_ptr<GLuint, FrameBufferDeleter> m_RendererIDPtr;
    std::vector<std::unique_ptr<GLuint, TextureDeleter>> m_Textures; // Stores texture IDs for color attachments
    std::unique_ptr<GLuint, RenderBufferDeleter> m_DepthRenderBufferPtr;

    // Store attachments for resizing
    std::vector<FrameBufferTextureAttachment> m_Attachments;

    int m_Width = 0;
    int m_Height = 0;
    bool m_HasDepth = false;
};