#pragma once
#include <glad/glad.h>

struct TextureConfig {
    GLenum internalFormat = GL_RGBA8;
    GLenum wrapS = GL_REPEAT;
    GLenum wrapT = GL_REPEAT;
    GLenum wrapR = GL_REPEAT; // Added for cube maps and arrays
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum magFilter = GL_LINEAR;
    bool generateMips = true;
    bool useAnisotropy = true;
    bool useBindless = false;
};