#pragma once
#include <glad/glad.h>

/**
 * @brief Configuration for texture creation: format, wrapping, filtering,
 *        and optional advanced features (mipmaps, anisotropy, bindless).
 *
 * Also includes flags for HDR and sRGB usage.
 */
struct TextureConfig {
    GLenum internalFormat = GL_RGBA8;
    GLenum wrapS = GL_REPEAT;
    GLenum wrapT = GL_REPEAT;
    GLenum wrapR = GL_REPEAT;  // For cube maps or 3D textures
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum magFilter = GL_LINEAR;
    bool   generateMips = true;
    bool   useAnisotropy = true;
    bool   useBindless = false;

    // Additional flags for HDR or sRGB
    bool isHDR = false;  ///< If true, we expect float data (e.g., from stbi_loadf).
    bool isSRGB = false;  ///< If true, use an sRGB internal format if LDR (e.g., GL_SRGB8_ALPHA8).
};