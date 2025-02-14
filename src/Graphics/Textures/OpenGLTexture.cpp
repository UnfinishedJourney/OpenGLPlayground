#include "OpenGLTexture.h"
#include "TextureUtils.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <cmath>

namespace graphics {

    OpenGLTexture::OpenGLTexture(const Bitmap& image, const TextureConfig& config) {
        width_ = static_cast<uint32_t>(image.width());
        height_ = static_cast<uint32_t>(image.height());

        glCreateTextures(GL_TEXTURE_2D, 1, &texture_id_);
        if (!texture_id_) throw std::runtime_error("OpenGLTexture: Failed to create texture!");

        GLenum finalFmt = ResolveInternalFormat(config, image.components());
        int levels = config.generate_mips_ ? static_cast<int>(std::floor(std::log2(std::max(width_, height_)))) + 1 : 1;
        glTextureStorage2D(texture_id_, levels, finalFmt, width_, height_);

        if (!image.data().empty()) {
            GLenum type = DataTypeForHDR(image.IsHDR());
            GLenum format = GL_RGBA;
            glTextureSubImage2D(texture_id_, 0, 0, 0, width_, height_, format, type,
                image.IsHDR() ? (const void*)image.GetDataFloat() : (const void*)image.GetDataU8());
        }

        SetupTextureParameters(texture_id_, config, false);
        MakeBindlessIfNeeded(config.use_bindless_);
    }

} // namespace graphics