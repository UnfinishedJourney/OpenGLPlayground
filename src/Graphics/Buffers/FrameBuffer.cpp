#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"  
#include <algorithm>
#include <utility>

namespace graphics {

    FrameBuffer::FrameBuffer(int width,
        int height,
        const std::vector<FrameBufferTextureAttachment>& attachments,
        bool hasDepth,
        int samples)
        : rendererId_(0),
        colorTextures_(),
        depthTexture_(0),
        attachments_(attachments),
        width_(width),
        height_(height),
        hasDepth_(hasDepth),
        samples_(samples)
    {
        Initialize();
    }

    FrameBuffer::~FrameBuffer() {
        Cleanup();
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
        : rendererId_(std::exchange(other.rendererId_, 0)),
        colorTextures_(std::move(other.colorTextures_)),
        depthTexture_(std::exchange(other.depthTexture_, 0)),
        attachments_(std::move(other.attachments_)),
        width_(std::exchange(other.width_, 0)),
        height_(std::exchange(other.height_, 0)),
        hasDepth_(other.hasDepth_),
        samples_(other.samples_)
    {
    }

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            Cleanup();
            rendererId_ = std::exchange(other.rendererId_, 0);
            colorTextures_ = std::move(other.colorTextures_);
            depthTexture_ = std::exchange(other.depthTexture_, 0);
            attachments_ = std::move(other.attachments_);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
            hasDepth_ = other.hasDepth_;
            samples_ = other.samples_;
        }
        return *this;
    }

    void FrameBuffer::Initialize() {
        GLCall(glCreateFramebuffers(1, &rendererId_));
        if (rendererId_ == 0) {
            Logger::GetLogger()->error("FrameBuffer: Failed to create framebuffer.");
            throw std::runtime_error("FrameBuffer: Failed to create framebuffer.");
        }
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, rendererId_));

        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(attachments_.size());

        for (const auto& att : attachments_) {
            GLuint textureId = CreateColorAttachment(att);
            colorTextures_.push_back(textureId);
            if (att.attachment_type >= GL_COLOR_ATTACHMENT0 && att.attachment_type <= GL_COLOR_ATTACHMENT15) {
                drawBuffers.push_back(att.attachment_type);
            }
            Logger::GetLogger()->info("FrameBuffer: Attached color texture (ID={}) to FBO={} (attachment=0x{:X}).",
                textureId, rendererId_, att.attachment_type);
        }

        if (!drawBuffers.empty()) {
            GLCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
        }
        else {
            GLCall(glDrawBuffer(GL_NONE));
            GLCall(glReadBuffer(GL_NONE));
        }

        if (hasDepth_) {
            depthTexture_ = CreateDepthAttachment();
            Logger::GetLogger()->info("FrameBuffer: Attached depth texture (ID={}) to FBO={}.",
                depthTexture_, rendererId_);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            Logger::GetLogger()->error("FrameBuffer (ID={}) is incomplete! Status=0x{:X}", rendererId_, status);
            throw std::runtime_error("FrameBuffer: Incomplete framebuffer!");
        }

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        Logger::GetLogger()->info("FrameBuffer: Initialized FBO (ID={}) with {} samples. Size={}x{}.",
            rendererId_, samples_, width_, height_);
    }

    GLuint FrameBuffer::CreateColorAttachment(const FrameBufferTextureAttachment& att) const {
        GLuint textureId = 0;
        if (samples_ > 1) {
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &textureId));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureId));
            GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples_,
                att.internal_format,
                width_, height_,
                GL_TRUE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachment_type,
                GL_TEXTURE_2D_MULTISAMPLE,
                textureId,
                0));
        }
        else {
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &textureId));
            GLCall(glBindTexture(GL_TEXTURE_2D, textureId));
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
                att.internal_format,
                width_, height_, 0,
                att.format,
                att.type,
                nullptr));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachment_type,
                GL_TEXTURE_2D,
                textureId,
                0));
        }
        return textureId;
    }

    GLuint FrameBuffer::CreateDepthAttachment() const {
        GLuint textureId = 0;
        GLenum target = (samples_ > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        GLCall(glCreateTextures(target, 1, &textureId));
        GLCall(glBindTexture(target, textureId));

        if (samples_ > 1) {
            GLCall(glTexImage2DMultisample(target,
                samples_,
                GL_DEPTH_COMPONENT24,
                width_, height_,
                GL_TRUE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                target,
                textureId,
                0));
        }
        else {
            GLCall(glTexImage2D(target, 0,
                GL_DEPTH_COMPONENT24,
                width_, height_, 0,
                GL_DEPTH_COMPONENT,
                GL_UNSIGNED_INT,
                nullptr));
            GLCall(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                target,
                textureId,
                0));
        }
        return textureId;
    }

    void FrameBuffer::Cleanup() {
        if (rendererId_ != 0) {
            Logger::GetLogger()->info("FrameBuffer: Deleting FBO (ID={}).", rendererId_);
            GLCall(glDeleteFramebuffers(1, &rendererId_));
            rendererId_ = 0;
        }
        for (GLuint tex : colorTextures_) {
            if (tex != 0) {
                Logger::GetLogger()->info("FrameBuffer: Deleting color texture (ID={}).", tex);
                GLCall(glDeleteTextures(1, &tex));
            }
        }
        colorTextures_.clear();
        if (depthTexture_ != 0) {
            Logger::GetLogger()->info("FrameBuffer: Deleting depth texture (ID={}).", depthTexture_);
            GLCall(glDeleteTextures(1, &depthTexture_));
            depthTexture_ = 0;
        }
    }

    void FrameBuffer::Bind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, rendererId_));
        GLCall(glViewport(0, 0, width_, height_));
    }

    void FrameBuffer::Unbind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    GLuint FrameBuffer::GetTexture(GLenum attachment) const {
        int index = attachment - GL_COLOR_ATTACHMENT0;
        if (index >= 0 && index < static_cast<int>(colorTextures_.size())) {
            return colorTextures_[index];
        }
        Logger::GetLogger()->error("FrameBuffer::GetTexture: Invalid attachment 0x{:X}.", attachment);
        return 0;
    }

    GLuint FrameBuffer::GetDepthTexture() const {
        return depthTexture_;
    }

    void FrameBuffer::Resize(int newWidth, int newHeight) {
        if (newWidth == width_ && newHeight == height_) {
            return;
        }
        width_ = newWidth;
        height_ = newHeight;
        Cleanup();
        Initialize();
        Logger::GetLogger()->info("FrameBuffer: Resized FBO (ID={}) to {}x{}.", rendererId_, width_, height_);
    }

    void FrameBuffer::BlitTo(const FrameBuffer& targetFbo, GLbitfield mask, GLenum filter) const {
        GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, rendererId_));
        GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFbo.GetRendererID()));
        GLCall(glBlitFramebuffer(
            0, 0, width_, height_,
            0, 0, targetFbo.GetWidth(), targetFbo.GetHeight(),
            mask,
            filter
        ));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

} // namespace graphics