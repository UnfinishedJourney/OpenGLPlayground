#pragma once

#include <glad/glad.h>

struct RenderState {
    bool depthTestEnabled = true;
    GLenum depthFunc = GL_LESS;
    bool depthMask = GL_TRUE;

    bool blendingEnabled = false;
    GLenum blendSrcFactor = GL_ONE;
    GLenum blendDstFactor = GL_ZERO;

    bool faceCullingEnabled = false;
    GLenum cullFace = GL_BACK;
    GLenum frontFace = GL_CCW;

    // Add more states as needed
};