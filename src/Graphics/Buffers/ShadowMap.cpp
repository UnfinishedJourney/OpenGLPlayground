#include "ShadowMap.h"
#include <glad/glad.h>
#include <iostream>

namespace Graphics {

    ShadowMap::ShadowMap(GLsizei width, GLsizei height)
        : m_Width(width), m_Height(height)
    {
        // Define a border color for texture clamping.
        GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };

        // Create and configure the depth texture.
        glGenTextures(1, &m_DepthTex);
        glBindTexture(GL_TEXTURE_2D, m_DepthTex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, m_Width, m_Height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

        // Create and configure the framebuffer.
        glGenFramebuffers(1, &m_ShadowFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTex, 0);
        GLenum drawBuffers[] = { GL_NONE };
        glDrawBuffers(1, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: ShadowMap FBO is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ShadowMap::~ShadowMap() {
        glDeleteFramebuffers(1, &m_ShadowFBO);
        glDeleteTextures(1, &m_DepthTex);
    }

    void ShadowMap::BindForWriting() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
        glViewport(0, 0, m_Width, m_Height);
    }

    void ShadowMap::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

} // namespace Graphics