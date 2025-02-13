#pragma once
#include <glad/glad.h>

namespace graphics {

    /**
     * @brief Configuration parameters for texture creation.
     *
     * Includes internal format, wrapping, filtering, mipmap generation,
     * anisotropy, bindless usage, and flags for HDR and sRGB.
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

        // Flags for HDR and sRGB usage.
        bool isHDR = false;  ///< Expect float data (e.g. stbi_loadf).
        bool isSRGB = false; ///< Use an sRGB internal format if LDR.
    };

} // namespace graphics