#pragma once

#include <glad/glad.h>

class RenderState {
public:
    RenderState();

    // State configuration methods
    void SetCullFace(bool enabled, GLenum cullFace = GL_BACK);
    void SetDepthTest(bool enabled);
    void SetDepthFunc(GLenum func);
    void SetBlend(bool enabled, GLenum sfactor = GL_SRC_ALPHA, GLenum dfactor = GL_ONE_MINUS_SRC_ALPHA);
    void SetDepthMask(bool enabled);
    void SetStencilTest(bool enabled);
    void SetStencilFunc(GLenum func, GLint ref, GLuint mask);
    void SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
    // Add more state configurations as needed

    void Apply();

private:
    // State variables
    bool m_CullFaceEnabled;
    GLenum m_CullFaceMode;

    bool m_DepthTestEnabled;
    GLenum m_DepthFunc;

    bool m_BlendEnabled;
    GLenum m_BlendSFactor;
    GLenum m_BlendDFactor;

    bool m_DepthMaskEnabled;

    bool m_StencilTestEnabled;
    GLenum m_StencilFunc;
    GLint m_StencilRef;
    GLuint m_StencilMask;
    GLenum m_StencilSFail;
    GLenum m_StencilDPFail;
    GLenum m_StencilDPPass;

    // Add more state variables as needed
};