#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

FrameBuffer::FrameBuffer(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth)
    : m_Width(width), m_Height(height), m_HasDepth(hasDepth) {
    Initialize(width, height, attachments, hasDepth);
}

FrameBuffer::~FrameBuffer() {
    Cleanup();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID),
    m_Width(other.m_Width),
    m_Height(other.m_Height),
    m_HasDepth(other.m_HasDepth),
    m_Textures(std::move(other.m_Textures)),
    m_DepthRenderBuffer(other.m_DepthRenderBuffer) {
    other.m_RendererID = 0;
    other.m_DepthRenderBuffer = 0;
    Logger::GetLogger()->debug("Moved FrameBuffer with ID {}.", m_RendererID);
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
    if (this != &other) {
        Cleanup();

        m_RendererID = other.m_RendererID;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_HasDepth = other.m_HasDepth;
        m_Textures = std::move(other.m_Textures);
        m_DepthRenderBuffer = other.m_DepthRenderBuffer;

        other.m_RendererID = 0;
        other.m_DepthRenderBuffer = 0;
        Logger::GetLogger()->debug("Assigned FrameBuffer with ID {}.", m_RendererID);
    }
    return *this;
}

void FrameBuffer::Bind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
    GLCall(glViewport(0, 0, m_Width, m_Height));
}

void FrameBuffer::Unbind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint FrameBuffer::GetTexture(GLenum attachment) const {
    // attachment should be GL_COLOR_ATTACHMENT0 + n
    int index = attachment - GL_COLOR_ATTACHMENT0;
    if (index >= 0 && index < static_cast<int>(m_Textures.size())) {
        return m_Textures[index];
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
    Initialize(newWidth, newHeight, /* Pass existing attachments */{}, m_HasDepth);
    Logger::GetLogger()->info("Resized FrameBuffer to {}x{}.", newWidth, newHeight);
}

void FrameBuffer::Initialize(int width, int height, const std::vector<FrameBufferTextureAttachment>& attachments, bool hasDepth) {
    GLCall(glCreateFramebuffers(1, &m_RendererID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

    std::vector<GLenum> drawBuffers;

    for (const auto& attachment : attachments) {
        GLuint texture;
        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &texture));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, attachment.internalFormat, width, height, 0, attachment.format, attachment.type, nullptr));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)); // Adjust as needed
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // Adjust as needed
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment.attachmentType, GL_TEXTURE_2D, texture, 0));
        m_Textures.push_back(texture);
        drawBuffers.push_back(attachment.attachmentType);
        Logger::GetLogger()->info("Attached texture ID {} to FrameBuffer ID {} as {}.", texture, m_RendererID, attachment.attachmentType);
    }

    if (!drawBuffers.empty()) {
        GLCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
    }
    else {
        // No color attachments, disable drawing to color buffer
        GLCall(glDrawBuffer(GL_NONE));
        GLCall(glReadBuffer(GL_NONE));
    }

    if (hasDepth) {
        GLCall(glCreateRenderbuffers(1, &m_DepthRenderBuffer));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderBuffer));
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRenderBuffer));
        Logger::GetLogger()->info("Attached Depth RenderBuffer ID {} to FrameBuffer ID {}.", m_DepthRenderBuffer, m_RendererID);
    }

    // **Corrected Section Starts Here**
    // Separate the GLCall from the function that returns a value
    GLenum status;
    GLCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    // **Corrected Section Ends Here**

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::GetLogger()->error("FrameBuffer ID {} is not complete!", m_RendererID);
        throw std::runtime_error("Incomplete FrameBuffer.");
    }

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    Logger::GetLogger()->info("Initialized FrameBuffer with ID {}.", m_RendererID);
}

void FrameBuffer::Cleanup() {
    if (m_RendererID != 0) {
        GLCall(glDeleteFramebuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted FrameBuffer with ID {}.", m_RendererID);
        m_RendererID = 0;
    }

    for (auto texture : m_Textures) {
        GLCall(glDeleteTextures(1, &texture));
        Logger::GetLogger()->info("Deleted FrameBuffer texture ID {}.", texture);
    }
    m_Textures.clear();

    if (m_DepthRenderBuffer != 0) {
        GLCall(glDeleteRenderbuffers(1, &m_DepthRenderBuffer));
        Logger::GetLogger()->info("Deleted Depth RenderBuffer ID {}.", m_DepthRenderBuffer);
        m_DepthRenderBuffer = 0;
    }
}