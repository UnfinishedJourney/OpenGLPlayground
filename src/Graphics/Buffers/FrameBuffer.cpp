#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // for GLCall macros, etc.
#include <stdexcept>

FrameBuffer::FrameBuffer(int width,
    int height,
    const std::vector<FrameBufferTextureAttachment>& attachments,
    bool hasDepth /*=true*/,
    int  samples  /*=1*/)
    : m_RendererIDPtr(std::make_unique<GLuint>(0).release(), FrameBufferDeleter())
    , m_Width(width)
    , m_Height(height)
    , m_HasDepth(hasDepth)
    , m_Samples(samples)
    , m_Attachments(attachments)
{
    if (m_HasDepth) {
        m_DepthRenderBufferPtr = std::unique_ptr<GLuint, RenderBufferDeleter>(
            std::make_unique<GLuint>(0).release(), RenderBufferDeleter());
    }
    Initialize(m_Width, m_Height, m_Attachments, m_HasDepth, m_Samples);
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

    // Create textures (multisample or single-sample) for each attachment
    for (const auto& att : attachments) {
        auto texturePtr = std::unique_ptr<GLuint, TextureDeleter>(
            std::make_unique<GLuint>(0).release(), TextureDeleter());

        if (samples > 1) {
            // Multisample texture
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *texturePtr));

            GLCall(glTexImage2DMultisample(
                GL_TEXTURE_2D_MULTISAMPLE,
                samples,
                att.internalFormat,
                width, height,
                GL_TRUE
            ));

            GLCall(glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D_MULTISAMPLE,
                *texturePtr,
                0
            ));
        }
        else {
            // Single-sample texture
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D, *texturePtr));

            GLCall(glTexImage2D(
                GL_TEXTURE_2D, 0,
                att.internalFormat,
                width, height, 0,
                att.format, att.type,
                nullptr
            ));

            // Typical filtering/wrapping for off-screen rendering
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            GLCall(glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D,
                *texturePtr,
                0
            ));
        }

        m_Textures.push_back(std::move(texturePtr));

        // Collect color attachments for glDrawBuffers
        if (att.attachmentType >= GL_COLOR_ATTACHMENT0 &&
            att.attachmentType <= GL_COLOR_ATTACHMENT15)
        {
            drawBuffers.push_back(att.attachmentType);
        }

        Logger::GetLogger()->info(
            "Attached Texture ID={} to FBO={} (attachment=0x{:X}).",
            *m_Textures.back(), *m_RendererIDPtr, att.attachmentType
        );
    }

    // Set draw buffers for color attachments
    if (!drawBuffers.empty()) {
        GLCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
    }
    else {
        // No color attachments
        GLCall(glDrawBuffer(GL_NONE));
        GLCall(glReadBuffer(GL_NONE));
    }

    // Depth renderbuffer (if requested)
    if (hasDepth && m_DepthRenderBufferPtr) {
        GLCall(glCreateRenderbuffers(1, m_DepthRenderBufferPtr.get()));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, *m_DepthRenderBufferPtr));

        if (samples > 1) {
            GLCall(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24,
                width, height));
        }
        else {
            GLCall(glRenderbufferStorage(GL_RENDERBUFFER,
                GL_DEPTH_COMPONENT24,
                width, height));
        }

        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            *m_DepthRenderBufferPtr));

        Logger::GetLogger()->info(
            "Attached Depth RBO ID={} to FBO={}.",
            *m_DepthRenderBufferPtr, *m_RendererIDPtr
        );
    }

    // Check FBO completeness
    GLenum status = GL_FRAMEBUFFER_COMPLETE;
    GLCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::GetLogger()->error(
            "FBO ID={} is incomplete! Status=0x{:X}",
            *m_RendererIDPtr, status
        );
        throw std::runtime_error("Incomplete FrameBuffer: glCheckFramebufferStatus failed.");
    }

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    Logger::GetLogger()->info(
        "Initialized FBO (ID={}) with {} samples. Width={}, Height={}.",
        *m_RendererIDPtr, samples, width, height
    );
}

void FrameBuffer::Cleanup()
{
    // By resetting the unique_ptrs, we rely on their custom deleters to free GPU resources
    if (m_RendererIDPtr && *m_RendererIDPtr != 0) {
        Logger::GetLogger()->info("Deleting FBO (ID={}).", *m_RendererIDPtr);
    }
    m_RendererIDPtr.reset();

    for (auto& texPtr : m_Textures) {
        if (texPtr && *texPtr != 0) {
            Logger::GetLogger()->info("Deleting Texture (ID={}).", *texPtr);
        }
    }
    m_Textures.clear();

    if (m_DepthRenderBufferPtr && *m_DepthRenderBufferPtr != 0) {
        Logger::GetLogger()->info("Deleting Depth RBO (ID={}).", *m_DepthRenderBufferPtr);
    }
    m_DepthRenderBufferPtr.reset();
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
    // Expect attachments like GL_COLOR_ATTACHMENT0 + n
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
        return; // No change
    }

    m_Width = newWidth;
    m_Height = newHeight;

    Cleanup();
    // Re-initialize with updated dimensions
    Initialize(m_Width, m_Height, m_Attachments, m_HasDepth, m_Samples);

    Logger::GetLogger()->info("Resized FBO (ID={}) to {}x{}.",
        *m_RendererIDPtr, newWidth, newHeight);
}

void FrameBuffer::BlitTo(FrameBuffer& targetFBO, GLbitfield mask, GLenum filter) const
{
    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_RendererIDPtr));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO.GetRendererID()));

    GLCall(glBlitFramebuffer(
        0, 0, m_Width, m_Height,                  // src rect
        0, 0, targetFBO.GetWidth(), targetFBO.GetHeight(), // dst rect
        mask,
        filter
    ));

    // Unbind everything
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}