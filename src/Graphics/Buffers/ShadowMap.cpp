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
        constexpr GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Create and configure the depth texture.
        GLCall(glGenTextures(1, &depthTexture_));
        GLCall(glBindTexture(GL_TEXTURE_2D, depthTexture_));
        GLCall(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width_, height_));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS));

        // Create and configure the framebuffer.
        GLCall(glGenFramebuffers(1, &rendererId_));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, rendererId_));
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture_, 0));
        constexpr GLenum drawBuffers[] = { GL_NONE };
        GLCall(glDrawBuffers(1, drawBuffers));

        // Verify framebuffer completeness.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::GetLogger()->error("ShadowMap: Framebuffer (ID={}) is not complete!", rendererId_);
            throw std::runtime_error("ShadowMap: Incomplete framebuffer!");
        }

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        Logger::GetLogger()->info("ShadowMap: Created FBO (ID={}) with depth texture (ID={}) at resolution {}x{}.",
            rendererId_, depthTexture_, width_, height_);
    }

    ShadowMap::~ShadowMap() {
        Cleanup();
    }

    ShadowMap::ShadowMap(ShadowMap&& other) noexcept
        : rendererId_(std::exchange(other.rendererId_, 0)),
        depthTexture_(std::exchange(other.depthTexture_, 0)),
        width_(std::exchange(other.width_, 0)),
        height_(std::exchange(other.height_, 0))
    {
    }

    ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept {
        if (this != &other) {
            Cleanup();
            rendererId_ = std::exchange(other.rendererId_, 0);
            depthTexture_ = std::exchange(other.depthTexture_, 0);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
        }
        return *this;
    }

    void ShadowMap::BindForWriting() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, rendererId_));
        GLCall(glViewport(0, 0, width_, height_));
    }

    void ShadowMap::Unbind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void ShadowMap::Cleanup() {
        if (rendererId_ != 0) {
            Logger::GetLogger()->info("ShadowMap: Deleting FBO (ID={}).", rendererId_);
            GLCall(glDeleteFramebuffers(1, &rendererId_));
            rendererId_ = 0;
        }
        if (depthTexture_ != 0) {
            Logger::GetLogger()->info("ShadowMap: Deleting depth texture (ID={}).", depthTexture_);
            GLCall(glDeleteTextures(1, &depthTexture_));
            depthTexture_ = 0;
        }
    }

} // namespace graphics