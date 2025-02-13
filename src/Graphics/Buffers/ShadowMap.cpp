#include "ShadowMap.h"
#include "Utilities/Utility.h"   // Contains the GLCall macro.
#include "Utilities/Logger.h"    // Logger for logging messages.
#include <stdexcept>
#include <utility>  // For std::exchange

namespace graphics {

    ShadowMap::ShadowMap(GLsizei width, GLsizei height)
        : width_(width), height_(height)
    {
        // Define a border color for texture clamping.
        constexpr GLfloat border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Create and configure the depth texture.
        GLCall(glGenTextures(1, &depth_texture_));
        GLCall(glBindTexture(GL_TEXTURE_2D, depth_texture_));
        GLCall(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width_, height_));
        // For non-multisample textures, filtering is used when sampling.
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color));
        // Set texture comparison mode for shadow mapping.
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS));

        // Create and configure the framebuffer.
        GLCall(glGenFramebuffers(1, &renderer_id_));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_));
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_, 0));
        // No color output for shadow maps.
        constexpr GLenum draw_buffers[] = { GL_NONE };
        GLCall(glDrawBuffers(1, draw_buffers));

        // Verify framebuffer completeness.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::GetLogger()->error("ShadowMap: Framebuffer (ID={}) is not complete!", renderer_id_);
            throw std::runtime_error("ShadowMap: Incomplete framebuffer!");
        }

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        Logger::GetLogger()->info("ShadowMap: Created FBO (ID={}) with depth texture (ID={}) at resolution {}x{}.",
            renderer_id_, depth_texture_, width_, height_);
    }

    ShadowMap::~ShadowMap() {
        Cleanup();
    }

    ShadowMap::ShadowMap(ShadowMap&& other) noexcept
        : renderer_id_(std::exchange(other.renderer_id_, 0)),
        depth_texture_(std::exchange(other.depth_texture_, 0)),
        width_(std::exchange(other.width_, 0)),
        height_(std::exchange(other.height_, 0))
    {}

    ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept {
        if (this != &other) {
            Cleanup();
            renderer_id_ = std::exchange(other.renderer_id_, 0);
            depth_texture_ = std::exchange(other.depth_texture_, 0);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
        }
        return *this;
    }

    void ShadowMap::BindForWriting() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_));
        GLCall(glViewport(0, 0, width_, height_));
    }

    void ShadowMap::Unbind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void ShadowMap::Cleanup() {
        if (renderer_id_ != 0) {
            Logger::GetLogger()->info("ShadowMap: Deleting FBO (ID={}).", renderer_id_);
            GLCall(glDeleteFramebuffers(1, &renderer_id_));
            renderer_id_ = 0;
        }
        if (depth_texture_ != 0) {
            Logger::GetLogger()->info("ShadowMap: Deleting depth texture (ID={}).", depth_texture_);
            GLCall(glDeleteTextures(1, &depth_texture_));
            depth_texture_ = 0;
        }
    }

} // namespace graphics