//#include "RenderState.h"
//
//RenderState::RenderState()
//    : m_CullFaceEnabled(false), m_CullFaceMode(GL_BACK),
//    m_DepthTestEnabled(true), m_DepthFunc(GL_LESS),
//    m_BlendEnabled(false), m_BlendSFactor(GL_SRC_ALPHA), m_BlendDFactor(GL_ONE_MINUS_SRC_ALPHA),
//    m_DepthMaskEnabled(true),
//    m_StencilTestEnabled(false), m_StencilFunc(GL_ALWAYS), m_StencilRef(0), m_StencilMask(0xFF),
//    m_StencilSFail(GL_KEEP), m_StencilDPFail(GL_KEEP), m_StencilDPPass(GL_KEEP)
//{
//}
//
//void RenderState::SetCullFace(bool enabled, GLenum cullFace)
//{
//    m_CullFaceEnabled = enabled;
//    m_CullFaceMode = cullFace;
//}
//
//void RenderState::SetDepthTest(bool enabled)
//{
//    m_DepthTestEnabled = enabled;
//}
//
//void RenderState::SetDepthFunc(GLenum func)
//{
//    m_DepthFunc = func;
//}
//
//void RenderState::SetBlend(bool enabled, GLenum sfactor, GLenum dfactor)
//{
//    m_BlendEnabled = enabled;
//    m_BlendSFactor = sfactor;
//    m_BlendDFactor = dfactor;
//}
//
//void RenderState::SetDepthMask(bool enabled)
//{
//    m_DepthMaskEnabled = enabled;
//}
//
//void RenderState::SetStencilTest(bool enabled)
//{
//    m_StencilTestEnabled = enabled;
//}
//
//void RenderState::SetStencilFunc(GLenum func, GLint ref, GLuint mask)
//{
//    m_StencilFunc = func;
//    m_StencilRef = ref;
//    m_StencilMask = mask;
//}
//
//void RenderState::SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
//{
//    m_StencilSFail = sfail;
//    m_StencilDPFail = dpfail;
//    m_StencilDPPass = dppass;
//}
//
//void RenderState::Apply()
//{
//    // Cull Face
//    if (m_CullFaceEnabled)
//    {
//        glEnable(GL_CULL_FACE);
//        glCullFace(m_CullFaceMode);
//    }
//    else
//    {
//        glDisable(GL_CULL_FACE);
//    }
//
//    // Depth Test
//    if (m_DepthTestEnabled)
//    {
//        glEnable(GL_DEPTH_TEST);
//        glDepthFunc(m_DepthFunc);
//    }
//    else
//    {
//        glDisable(GL_DEPTH_TEST);
//    }
//
//    // Depth Mask
//    glDepthMask(m_DepthMaskEnabled ? GL_TRUE : GL_FALSE);
//
//    // Blending
//    if (m_BlendEnabled)
//    {
//        glEnable(GL_BLEND);
//        glBlendFunc(m_BlendSFactor, m_BlendDFactor);
//    }
//    else
//    {
//        glDisable(GL_BLEND);
//    }
//
//    // Stencil Test
//    if (m_StencilTestEnabled)
//    {
//        glEnable(GL_STENCIL_TEST);
//        glStencilFunc(m_StencilFunc, m_StencilRef, m_StencilMask);
//        glStencilOp(m_StencilSFail, m_StencilDPFail, m_StencilDPPass);
//    }
//    else
//    {
//        glDisable(GL_STENCIL_TEST);
//    }
//
//    // Apply other states as needed
//}