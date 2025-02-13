#pragma once
#include "GLBaseTexture.h"
#include "Bitmap.h"
#include "TextureConfig.h"
#include <string>
#include <vector>

namespace graphics {

    /**
     * @brief A 2D texture array built from slicing a sprite sheet.
     */
    class OpenGLTextureArray : public GLBaseTexture
    {
    public:
        OpenGLTextureArray(const std::vector<std::string>& filePaths,
            const TextureConfig& config,
            uint32_t totalFrames = 64,
            uint32_t gridX = 8,
            uint32_t gridY = 8);

        ~OpenGLTextureArray() override = default;
    };

} // namespace graphics