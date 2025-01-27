#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"   // for GLCall macros, etc.
#include <stdexcept>

FrameBuffer::FrameBuffer(int width,
    int height,
    const std::vector<FrameBufferTextureAttachment>& attachments,
    bool hasDepth /*=true*/,
    int samples   /*=1*/)
    : m_RendererIDPtr(new GLuint(0), FrameBufferDeleter())
    , m_DepthRenderBufferPtr(nullptr)
    , m_Width(width)
    , m_Height(height)
    , m_HasDepth(hasDepth)
    , m_Samples(samples)
    , m_Attachments(attachments)
{
    // Create a depth renderbuffer only if hasDepth == true
    if (m_HasDepth) {
        m_DepthRenderBufferPtr = std::unique_ptr<GLuint, RenderBufferDeleter>(new GLuint(0), RenderBufferDeleter());
    }

    Initialize(width, height, attachments, hasDepth, samples);
}

FrameBuffer::~FrameBuffer()
{
    Cleanup();
}

void FrameBuffer::Initialize(int width,
    int height,
    const std::vector<FrameBufferTextureAttachment>& attachments,
    bool hasDepth,
    int samples)
{
    GLCall(glCreateFramebuffers(1, m_RendererIDPtr.get()));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, *m_RendererIDPtr));

    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(attachments.size());

    for (const auto& att : attachments) {
        // We create one texture (or textureMS) per attachment
        auto texturePtr = std::unique_ptr<GLuint, TextureDeleter>(new GLuint(0), TextureDeleter());

        if (samples > 1) {
            // ----- MULTISAMPLE TEXTURE -----
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *texturePtr));

            // NOTE: No glTexParameteri() calls for sampler/filtering needed; MS textures ignore those.
            // We can fix the number of samples, and set the 'fixedSampleLocations' to GL_TRUE for typical usage.
            GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples,
                att.internalFormat, // e.g. GL_RGBA8
                width, height,
                GL_TRUE));
            // Attach it to FBO
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D_MULTISAMPLE,
                *texturePtr,
                0));
        }
        else {
            // ----- SINGLE-SAMPLE TEXTURE -----
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D, *texturePtr));

            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, att.internalFormat,
                width, height, 0,
                att.format, att.type, nullptr));

            // Typical filtering/wrapping for an off-screen color texture
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            // Attach to FBO
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D,
                *texturePtr,
                0));
        }

        m_Textures.push_back(std::move(texturePtr));
        if (att.attachmentType >= GL_COLOR_ATTACHMENT0 &&
            att.attachmentType <= GL_COLOR_ATTACHMENT15)
        {
            drawBuffers.push_back(att.attachmentType);
        }

        Logger::GetLogger()->info("Attached Texture ID {} to FBO ID {} (attachment={}).",
            *m_Textures.back(),
            *m_RendererIDPtr,
            att.attachmentType);
    }

    // Set the color draw buffers if we have color attachments
    if (!drawBuffers.empty()) {
        GLCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
    }
    else {
        GLCall(glDrawBuffer(GL_NONE));
        GLCall(glReadBuffer(GL_NONE));
    }

    // Depth (as renderbuffer or as a separate texture). We'll do a renderbuffer for simplicity:
    if (hasDepth && m_DepthRenderBufferPtr) {
        GLCall(glCreateRenderbuffers(1, m_DepthRenderBufferPtr.get()));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, *m_DepthRenderBufferPtr));

        if (samples > 1) {
            // multisample
            GLCall(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                samples,
                GL_DEPTH_COMPONENT24,
                width, height));
        }
        else {
            // single-sample
            GLCall(glRenderbufferStorage(GL_RENDERBUFFER,
                GL_DEPTH_COMPONENT24,
                width, height));
        }
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            *m_DepthRenderBufferPtr));

        Logger::GetLogger()->info("Attached Depth RBO ID {} to FBO ID {}.",
            *m_DepthRenderBufferPtr, *m_RendererIDPtr);
    }

    // Check completeness
    GLenum status;
    GLCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::GetLogger()->error("FBO ID {} is incomplete! Status: 0x{:X}", *m_RendererIDPtr, status);
        throw std::runtime_error("Incomplete FrameBuffer.");
    }

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    Logger::GetLogger()->info("Initialized FBO (ID={}) with {} samples.", *m_RendererIDPtr, samples);
}

void FrameBuffer::Cleanup()
{
    // Delete FBO
    if (*m_RendererIDPtr != 0) {
        GLCall(glDeleteFramebuffers(1, m_RendererIDPtr.get()));
        Logger::GetLogger()->info("Deleted FBO (ID={}).", *m_RendererIDPtr);
        *m_RendererIDPtr = 0;
    }

    // Delete color / attachments
    for (auto& texPtr : m_Textures) {
        if (*texPtr != 0) {
            GLCall(glDeleteTextures(1, texPtr.get()));
            Logger::GetLogger()->info("Deleted FBO Texture (ID={}).", *texPtr);
            *texPtr = 0;
        }
    }
    m_Textures.clear();

    // Delete RBO
    if (m_DepthRenderBufferPtr && *m_DepthRenderBufferPtr != 0) {
        GLCall(glDeleteRenderbuffers(1, m_DepthRenderBufferPtr.get()));
        Logger::GetLogger()->info("Deleted Depth RBO (ID={}).", *m_DepthRenderBufferPtr);
        *m_DepthRenderBufferPtr = 0;
    }
}

void FrameBuffer::Bind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, *m_RendererIDPtr));
    GLCall(glViewport(0, 0, m_Width, m_Height));
}

void FrameBuffer::Unbind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint FrameBuffer::GetTexture(GLenum attachment) const
{
    // NOTE: This is relevant only if this FBO is single-sample or you want the MS texture ID (not typically sampled).
    // attachment should be something like GL_COLOR_ATTACHMENT0 + n
    int index = attachment - GL_COLOR_ATTACHMENT0;
    if (index >= 0 && static_cast<size_t>(index) < m_Textures.size()) {
        return *m_Textures[index];
    }
    Logger::GetLogger()->error("Invalid attachment type queried: 0x{:X}", attachment);
    return 0;
}

void FrameBuffer::Resize(int newWidth, int newHeight)
{
    if (newWidth == m_Width && newHeight == m_Height) {
        return;
    }
    m_Width = newWidth;
    m_Height = newHeight;

    Cleanup();
    Initialize(m_Width, m_Height, m_Attachments, m_HasDepth, m_Samples);
    Logger::GetLogger()->info("Resized FBO (ID={}) to {}x{}.", *m_RendererIDPtr, newWidth, newHeight);
}

void FrameBuffer::BlitTo(FrameBuffer& targetFBO, GLbitfield mask, GLenum filter) const
{
    // Read from this FBO
    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_RendererIDPtr));
    // Draw to target
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO.GetRendererID()));

    GLCall(glBlitFramebuffer(
        0, 0, m_Width, m_Height,   // src rect
        0, 0, targetFBO.GetWidth(), targetFBO.GetHeight(), // dst rect
        mask,  // which buffers to blit
        filter // filtering mode
    ));

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}