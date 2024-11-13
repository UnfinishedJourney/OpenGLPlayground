#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>
#include "Utilities/Logger.h"

struct FrameBufferTextureAttachment {
    GLenum attachmentType; // e.g., GL_COLOR_ATTACHMENT0
    GLenum internalFormat; // e.g., GL_RGBA8
    GLenum format;         // e.g., GL_RGBA
    GLenum type;           // e.g., GL_UNSIGNED_BYTE
};

class FrameBuffer {
public:
    FrameBuffer(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth = true);
    ~FrameBuffer();

    // Delete copy constructor and assignment operator
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    // Allow move semantics
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    // Get the texture ID for a specific attachment
    GLuint GetTexture(GLenum attachment) const;

    // Resize the framebuffer and its attachments
    void Resize(int newWidth, int newHeight);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }

private:
    void Initialize(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth);
    void Cleanup();

    GLuint m_RendererID = 0;
    int m_Width = 0;
    int m_Height = 0;
    bool m_HasDepth = false;
    std::vector<GLuint> m_Textures; // Stores texture IDs for color attachments
    GLuint m_DepthRenderBuffer = 0;
};