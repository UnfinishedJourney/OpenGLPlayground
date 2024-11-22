#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

FrameBuffer::FrameBuffer(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth)
    : m_Width(width), m_Height(height), m_HasDepth(hasDepth),
    m_RendererIDPtr(new GLuint(0), FrameBufferDeleter()),
    m_DepthRenderBufferPtr(hasDepth ? std::unique_ptr<GLuint, RenderBufferDeleter>(new GLuint(0), RenderBufferDeleter()) : nullptr),
    m_Attachments(attachments) // Store the attachments
{
    Initialize(width, height, attachments, hasDepth);
}

void FrameBuffer::Initialize(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth) {
    // Create FrameBuffer Object
    GLCall(glCreateFramebuffers(1, m_RendererIDPtr.get()));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, *m_RendererIDPtr));

    std::vector<GLenum> drawBuffers;

    // Create and attach textures
    for (const auto& attachment : attachments) {
        auto texturePtr = std::unique_ptr<GLuint, TextureDeleter>(new GLuint(0), TextureDeleter());
        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, texturePtr.get()));
        GLCall(glBindTexture(GL_TEXTURE_2D, *texturePtr));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, attachment.internalFormat, width, height, 0, attachment.format, attachment.type, nullptr));
        // Set texture parameters (adjust as needed)
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        // Attach texture to framebuffer
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment.attachmentType, GL_TEXTURE_2D, *texturePtr, 0));
        m_Textures.emplace_back(std::move(texturePtr));
        drawBuffers.push_back(attachment.attachmentType);

        Logger::GetLogger()->info("Attached Texture ID {} to FrameBuffer ID {} as attachment {}.",
            *m_Textures.back(), *m_RendererIDPtr, attachment.attachmentType);
    }

    // Specify the list of color buffers to draw to
    if (!drawBuffers.empty()) {
        GLCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
    }
    else {
        // No color attachments, disable color buffer drawing
        GLCall(glDrawBuffer(GL_NONE));
        GLCall(glReadBuffer(GL_NONE));
    }

    // Create and attach RenderBuffer for depth if required
    if (hasDepth && m_DepthRenderBufferPtr) {
        GLCall(glCreateRenderbuffers(1, m_DepthRenderBufferPtr.get()));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, *m_DepthRenderBufferPtr));
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *m_DepthRenderBufferPtr));

        Logger::GetLogger()->info("Attached Depth RenderBuffer ID {} to FrameBuffer ID {}.",
            *m_DepthRenderBufferPtr, *m_RendererIDPtr);
    }

    // Check framebuffer completeness
    GLenum status;
    GLCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::GetLogger()->error("FrameBuffer ID {} is incomplete! Status: {}", *m_RendererIDPtr, status);
        throw std::runtime_error("Incomplete FrameBuffer.");
    }

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    Logger::GetLogger()->info("Initialized FrameBuffer with ID {}.", *m_RendererIDPtr);
}

void FrameBuffer::Cleanup() {
    // Delete FrameBuffer Object
    if (*m_RendererIDPtr != 0) {
        GLCall(glDeleteFramebuffers(1, m_RendererIDPtr.get()));
        Logger::GetLogger()->info("Deleted FrameBuffer with ID {}.", *m_RendererIDPtr);
        *m_RendererIDPtr = 0;
    }

    // Delete Textures
    for (auto& texturePtr : m_Textures) {
        if (*texturePtr != 0) {
            GLCall(glDeleteTextures(1, texturePtr.get()));
            Logger::GetLogger()->info("Deleted Texture ID {}.", *texturePtr);
            *texturePtr = 0;
        }
    }
    m_Textures.clear();

    // Delete RenderBuffer
    if (m_DepthRenderBufferPtr && *m_DepthRenderBufferPtr != 0) {
        GLCall(glDeleteRenderbuffers(1, m_DepthRenderBufferPtr.get()));
        Logger::GetLogger()->info("Deleted Depth RenderBuffer ID {}.", *m_DepthRenderBufferPtr);
        *m_DepthRenderBufferPtr = 0;
    }
}

void FrameBuffer::Bind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, *m_RendererIDPtr));
    GLCall(glViewport(0, 0, m_Width, m_Height));
}

void FrameBuffer::Unbind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint FrameBuffer::GetTexture(GLenum attachment) const {
    // attachment should be GL_COLOR_ATTACHMENT0 + n
    int index = attachment - GL_COLOR_ATTACHMENT0;
    if (index >= 0 && static_cast<size_t>(index) < m_Textures.size()) {
        return *m_Textures[index];
    }
    Logger::GetLogger()->error("Invalid attachment type queried: {}.", attachment);
    return 0;
}

void FrameBuffer::Resize(int newWidth, int newHeight) {
    if (newWidth == m_Width && newHeight == m_Height)
        return;

    m_Width = newWidth;
    m_Height = newHeight;

    Cleanup();
    Initialize(newWidth, newHeight, m_Attachments, m_HasDepth); // Pass stored attachments
    Logger::GetLogger()->info("Resized FrameBuffer to {}x{}.", newWidth, newHeight);
}