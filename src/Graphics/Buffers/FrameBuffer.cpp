#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // GLCall macros, etc.
#include <stdexcept>

FrameBuffer::FrameBuffer(int width,
    int height,
    const std::vector<FrameBufferTextureAttachment>& attachments,
    bool hasDepth,
    int samples)
    : m_RendererIDPtr(std::make_unique<GLuint>(0).release(), FrameBufferDeleter())
    , m_Width(width)
    , m_Height(height)
    , m_HasDepth(hasDepth)
    , m_Samples(samples)
    , m_Attachments(attachments)
{
    // Depth texture is optional, create a pointer if requested
    if (m_HasDepth) {
        m_DepthTexturePtr = std::unique_ptr<GLuint, TextureDeleter>(
            std::make_unique<GLuint>(0).release(), TextureDeleter());
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

    // Create color textures
    for (const auto& att : attachments)
    {
        // Make a new texture handle
        auto texturePtr = std::unique_ptr<GLuint, TextureDeleter>(
            std::make_unique<GLuint>(0).release(), TextureDeleter());

        if (samples > 1)
        {
            // Multisample color texture
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *texturePtr));
            GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples,
                att.internalFormat,
                width, height,
                GL_TRUE));

            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D_MULTISAMPLE,
                *texturePtr,
                0));
        }
        else
        {
            // Single-sample color texture
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, texturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D, *texturePtr));

            GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
                att.internalFormat,
                width, height, 0,
                att.format,
                att.type,
                nullptr));

            // Typical filtering/wrapping for off-screen
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachmentType,
                GL_TEXTURE_2D,
                *texturePtr,
                0));
        }

        m_ColorTextures.push_back(std::move(texturePtr));

        // Collect color attachments for glDrawBuffers
        if (att.attachmentType >= GL_COLOR_ATTACHMENT0 &&
            att.attachmentType <= GL_COLOR_ATTACHMENT15)
        {
            drawBuffers.push_back(att.attachmentType);
        }

        Logger::GetLogger()->info(
            "Attached color Texture ID={} to FBO={} (attachment=0x{:X}).",
            *m_ColorTextures.back(), *m_RendererIDPtr, att.attachmentType
        );
    }

    // If we have color attachments, set them
    if (!drawBuffers.empty()) {
        GLCall(glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data()));
    }
    else {
        // No color attachments
        GLCall(glDrawBuffer(GL_NONE));
        GLCall(glReadBuffer(GL_NONE));
    }

    // ---------- Create & attach Depth Texture if requested -----------
    if (hasDepth && m_DepthTexturePtr)
    {
        if (samples > 1)
        {
            // Multisample depth texture
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, m_DepthTexturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *m_DepthTexturePtr));
            GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples,
                GL_DEPTH_COMPONENT24,
                width, height,
                GL_TRUE));

            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D_MULTISAMPLE,
                *m_DepthTexturePtr,
                0));
        }
        else
        {
            // Single-sample depth texture
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, m_DepthTexturePtr.get()));
            GLCall(glBindTexture(GL_TEXTURE_2D, *m_DepthTexturePtr));

            GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
                GL_DEPTH_COMPONENT24,
                width, height, 0,
                GL_DEPTH_COMPONENT,
                GL_UNSIGNED_INT,
                nullptr));

            // Typical depth parameters
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D,
                *m_DepthTexturePtr,
                0));
        }

        Logger::GetLogger()->info("Attached Depth Texture ID={} to FBO={}.",
            *m_DepthTexturePtr, *m_RendererIDPtr);
    }
    // --------------------------------------------------------------

    // Check FBO completeness
    GLenum status = GL_FRAMEBUFFER_COMPLETE;
    GLCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::GetLogger()->error("FBO ID={} is incomplete! Status=0x{:X}",
            *m_RendererIDPtr, status);
        throw std::runtime_error("Incomplete FrameBuffer!");
    }

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    Logger::GetLogger()->info(
        "Initialized FBO (ID={}) with {} samples. Width={}, Height={}.",
        *m_RendererIDPtr, samples, width, height
    );
}

void FrameBuffer::Cleanup()
{
    if (m_RendererIDPtr && *m_RendererIDPtr != 0) {
        Logger::GetLogger()->info("Deleting FBO (ID={}).", *m_RendererIDPtr);
    }
    m_RendererIDPtr.reset();

    for (auto& texPtr : m_ColorTextures) {
        if (texPtr && *texPtr != 0) {
            Logger::GetLogger()->info("Deleting Color Texture (ID={}).", *texPtr);
        }
    }
    m_ColorTextures.clear();

    if (m_DepthTexturePtr && *m_DepthTexturePtr != 0) {
        Logger::GetLogger()->info("Deleting Depth Texture (ID={}).", *m_DepthTexturePtr);
    }
    m_DepthTexturePtr.reset();
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
    if (index >= 0 && index < (int)m_ColorTextures.size()) {
        return *m_ColorTextures[index];
    }
    Logger::GetLogger()->error("GetTexture: Invalid attachment type queried: 0x{:X}", attachment);
    return 0;
}

GLuint FrameBuffer::GetDepthTexture() const
{
    if (!m_DepthTexturePtr) return 0;
    return *m_DepthTexturePtr;
}

void FrameBuffer::Resize(int newWidth, int newHeight)
{
    if (newWidth == m_Width && newHeight == m_Height) {
        return; // No change
    }

    m_Width = newWidth;
    m_Height = newHeight;

    Cleanup();
    Initialize(m_Width, m_Height, m_Attachments, m_HasDepth, m_Samples);

    Logger::GetLogger()->info("Resized FBO (ID={}) to {}x{}.",
        *m_RendererIDPtr, newWidth, newHeight);
}

void FrameBuffer::BlitTo(FrameBuffer& targetFBO, GLbitfield mask, GLenum filter) const
{
    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_RendererIDPtr));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO.GetRendererID()));

    GLCall(glBlitFramebuffer(
        0, 0, m_Width, m_Height,                    // src rect
        0, 0, targetFBO.GetWidth(), targetFBO.GetHeight(),  // dst rect
        mask,
        filter
    ));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}