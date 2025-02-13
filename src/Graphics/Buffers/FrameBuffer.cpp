#include "FrameBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"  // Contains GLCall macro.
#include <algorithm>
#include <stdexcept>
#include <utility> // For std::exchange

namespace graphics {

    FrameBuffer::FrameBuffer(int width,
        int height,
        const std::vector<FrameBufferTextureAttachment>& attachments,
        bool has_depth,
        int samples)
        : renderer_id_{ 0 },
        color_textures_{},
        depth_texture_{ 0 },
        attachments_{ attachments },
        width_{ width },
        height_{ height },
        has_depth_{ has_depth },
        samples_{ samples }
    {
        Initialize();
    }

    FrameBuffer::~FrameBuffer() {
        Cleanup();
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
        : renderer_id_{ std::exchange(other.renderer_id_, 0) },
        color_textures_{ std::move(other.color_textures_) },
        depth_texture_{ std::exchange(other.depth_texture_, 0) },
        attachments_{ std::move(other.attachments_) },
        width_{ std::exchange(other.width_, 0) },
        height_{ std::exchange(other.height_, 0) },
        has_depth_{ other.has_depth_ },
        samples_{ other.samples_ }
    {}

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            Cleanup();
            renderer_id_ = std::exchange(other.renderer_id_, 0);
            color_textures_ = std::move(other.color_textures_);
            depth_texture_ = std::exchange(other.depth_texture_, 0);
            attachments_ = std::move(other.attachments_);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
            has_depth_ = other.has_depth_;
            samples_ = other.samples_;
        }
        return *this;
    }

    void FrameBuffer::Initialize() {
        GLCall(glCreateFramebuffers(1, &renderer_id_));
        if (renderer_id_ == 0) {
            Logger::GetLogger()->error("FrameBuffer: Failed to create framebuffer.");
            throw std::runtime_error("FrameBuffer: Failed to create framebuffer.");
        }
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_));

        std::vector<GLenum> draw_buffers;
        draw_buffers.reserve(attachments_.size());

        // Create and attach color textures.
        for (const auto& att : attachments_) {
            GLuint texture_id = CreateColorAttachment(att);
            color_textures_.push_back(texture_id);
            if (att.attachment_type >= GL_COLOR_ATTACHMENT0 && att.attachment_type <= GL_COLOR_ATTACHMENT15) {
                draw_buffers.push_back(att.attachment_type);
            }
            Logger::GetLogger()->info("FrameBuffer: Attached color texture (ID={}) to FBO={} (attachment=0x{:X}).",
                texture_id, renderer_id_, att.attachment_type);
        }

        // Set draw buffers if any color attachments are present.
        if (!draw_buffers.empty()) {
            GLCall(glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()));
        }
        else {
            GLCall(glDrawBuffer(GL_NONE));
            GLCall(glReadBuffer(GL_NONE));
        }

        // Create and attach a depth texture if requested.
        if (has_depth_) {
            depth_texture_ = CreateDepthAttachment();
            Logger::GetLogger()->info("FrameBuffer: Attached depth texture (ID={}) to FBO={}.",
                depth_texture_, renderer_id_);
        }

        // Verify framebuffer completeness.
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            Logger::GetLogger()->error("FrameBuffer (ID={}) is incomplete! Status=0x{:X}", renderer_id_, status);
            throw std::runtime_error("FrameBuffer: Incomplete framebuffer!");
        }

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        Logger::GetLogger()->info("FrameBuffer: Initialized FBO (ID={}) with {} samples. Size={}x{}.",
            renderer_id_, samples_, width_, height_);
    }

    GLuint FrameBuffer::CreateColorAttachment(const FrameBufferTextureAttachment& att) const {
        GLuint texture_id = 0;
        if (samples_ > 1) {
            // Create multisample texture.
            GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texture_id));
            GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id));
            GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples_,
                att.internal_format,
                width_, height_,
                GL_TRUE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachment_type,
                GL_TEXTURE_2D_MULTISAMPLE,
                texture_id,
                0));
        }
        else {
            // Create standard 2D texture.
            GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &texture_id));
            GLCall(glBindTexture(GL_TEXTURE_2D, texture_id));
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
                att.internal_format,
                width_, height_, 0,
                att.format,
                att.type,
                nullptr));
            // Set common texture parameters.
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                att.attachment_type,
                GL_TEXTURE_2D,
                texture_id,
                0));
        }
        return texture_id;
    }

    GLuint FrameBuffer::CreateDepthAttachment() const {
        GLuint texture_id = 0;
        GLenum target = (samples_ > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        GLCall(glCreateTextures(target, 1, &texture_id));
        GLCall(glBindTexture(target, texture_id));

        if (samples_ > 1) {
            GLCall(glTexImage2DMultisample(target,
                samples_,
                GL_DEPTH_COMPONENT24,
                width_, height_,
                GL_TRUE));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                target,
                texture_id,
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
                texture_id,
                0));
        }
        return texture_id;
    }

    void FrameBuffer::Cleanup() {
        if (renderer_id_ != 0) {
            Logger::GetLogger()->info("FrameBuffer: Deleting FBO (ID={}).", renderer_id_);
            GLCall(glDeleteFramebuffers(1, &renderer_id_));
            renderer_id_ = 0;
        }
        for (GLuint texture_id : color_textures_) {
            if (texture_id != 0) {
                Logger::GetLogger()->info("FrameBuffer: Deleting color texture (ID={}).", texture_id);
                GLCall(glDeleteTextures(1, &texture_id));
            }
        }
        color_textures_.clear();

        if (depth_texture_ != 0) {
            Logger::GetLogger()->info("FrameBuffer: Deleting depth texture (ID={}).", depth_texture_);
            GLCall(glDeleteTextures(1, &depth_texture_));
            depth_texture_ = 0;
        }
    }

    void FrameBuffer::Bind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_));
        GLCall(glViewport(0, 0, width_, height_));
    }

    void FrameBuffer::Unbind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    GLuint FrameBuffer::GetTexture(GLenum attachment) const {
        int index = attachment - GL_COLOR_ATTACHMENT0;
        if (index >= 0 && index < static_cast<int>(color_textures_.size())) {
            return color_textures_[index];
        }
        Logger::GetLogger()->error("FrameBuffer::GetTexture: Invalid attachment 0x{:X}.", attachment);
        return 0;
    }

    GLuint FrameBuffer::GetDepthTexture() const {
        return depth_texture_;
    }

    void FrameBuffer::Resize(int new_width, int new_height) {
        if (new_width == width_ && new_height == height_) {
            return;
        }
        width_ = new_width;
        height_ = new_height;
        Cleanup();
        Initialize();
        Logger::GetLogger()->info("FrameBuffer: Resized FBO (ID={}) to {}x{}.", renderer_id_, width_, height_);
    }

    void FrameBuffer::BlitTo(const FrameBuffer& target_fbo, GLbitfield mask, GLenum filter) const {
        GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer_id_));
        GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target_fbo.GetRendererID()));
        GLCall(glBlitFramebuffer(
            0, 0, width_, height_,
            0, 0, target_fbo.GetWidth(), target_fbo.GetHeight(),
            mask,
            filter
        ));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

} // namespace graphics