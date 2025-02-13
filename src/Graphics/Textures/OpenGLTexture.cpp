#include "OpenGLTexture.h"
#include "TextureUtils.h"  // ResolveInternalFormat, DataTypeForHDR, SetupTextureParameters
#include "Utilities/Utility.h"
#include <cmath>
#include <stdexcept>

namespace graphics {

    OpenGLTexture::OpenGLTexture(const Bitmap& image, const TextureConfig& config)
    {
        width_ = static_cast<uint32_t>(image.width());
        height_ = static_cast<uint32_t>(image.height());

        // Create texture object
        glCreateTextures(GL_TEXTURE_2D, 1, &texture_id_);
        if (!texture_id_) {
            throw std::runtime_error("OpenGLTexture: Failed to create GL texture2D!");
        }

        // Decide final format
        GLenum finalFmt = ResolveInternalFormat(config, image.components());

        // Mip levels
        int levels = config.generate_mips_
            ? static_cast<int>(std::floor(std::log2(std::max(width_, height_)))) + 1
            : 1;

        // Allocate storage
        glTextureStorage2D(texture_id_, levels, finalFmt, width_, height_);

        // Upload if data non-empty
        if (!image.data().empty()) {
            GLenum type = DataTypeForHDR(image.IsHDR());
            GLenum format = GL_RGBA; // or GL_RGB if components=3, etc. 
            // For simplicity, if force4Ch was true, we do RGBA.

            glTextureSubImage2D(texture_id_, 0,
                0, 0,
                width_, height_,
                format,
                type,
                image.IsHDR() ? (const void*)image.GetDataFloat()
                : (const void*)image.GetDataU8());
        }

        // Setup parameters (wrap, filter, etc.)
        SetupTextureParameters(texture_id_, config, /*isCubeMap=*/false);

        // Make it bindless if requested
        MakeBindlessIfNeeded(config.use_bindless_);
    }

} // namespace graphics