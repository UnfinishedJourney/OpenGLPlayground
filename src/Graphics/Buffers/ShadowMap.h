#pragma once
#include <glad/glad.h>
#include <iostream>

class ShadowMap {
public:
    // Construct a shadow map with the given resolution.
    ShadowMap(GLsizei width, GLsizei height)
        : m_Width(width), m_Height(height)
    {
        // Border color for texture clamping
        GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };

        // Create the depth texture
        glGenTextures(1, &m_DepthTex);
        glBindTexture(GL_TEXTURE_2D, m_DepthTex);
        // Allocate storage for the texture – one mip level, 24-bit depth
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, m_Width, m_Height);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Set up shadow comparison mode (optional)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

        // Create and configure the FBO
        glGenFramebuffers(1, &m_ShadowFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTex, 0);
        // We don’t need any color output
        GLenum drawBuffers[] = { GL_NONE };
        glDrawBuffers(1, drawBuffers);

        // Check completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: ShadowMap FBO is not complete!" << std::endl;
        }
        // Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~ShadowMap() {
        glDeleteFramebuffers(1, &m_ShadowFBO);
        glDeleteTextures(1, &m_DepthTex);
    }

    void BindForWriting() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
        glViewport(0, 0, m_Width, m_Height);
    }

    void Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint GetDepthTexture() const { return m_DepthTex; }
    GLsizei GetWidth() const { return m_Width; }
    GLsizei GetHeight() const { return m_Height; }

private:
    GLuint m_ShadowFBO = 0;
    GLuint m_DepthTex = 0;
    GLsizei m_Width, m_Height;
};