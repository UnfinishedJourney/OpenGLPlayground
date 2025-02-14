#include "OpenGLTextureArray.h"
#include "TextureUtils.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <cmath>
#include <cstring>

namespace graphics {

    OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& filePaths,
        const TextureConfig& config,
        uint32_t totalFrames,
        uint32_t gridX,
        uint32_t gridY)
    {
        if (filePaths.empty()) {
            throw std::runtime_error("OpenGLTextureArray: no input files provided!");
        }
        if (filePaths.size() > 1) {
            throw std::runtime_error("OpenGLTextureArray: multiple files not supported in this example!");
        }

        // 1) Load the sprite sheet into a Bitmap
        Bitmap sheet;
        sheet.LoadFromFile(filePaths[0], /*flipY=*/false, /*force4Ch=*/true);

        width_ = sheet.width();
        height_ = sheet.height();
        int chans = sheet.components();

        // each frame dimension
        uint32_t frameW = width_ / gridX;
        uint32_t frameH = height_ / gridY;

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_id_);
        if (!texture_id_) {
            throw std::runtime_error("OpenGLTextureArray: glCreateTextures failed!");
        }

        // final format
        GLenum finalFmt = ResolveInternalFormat(config, chans);

        int levels = config.generate_mips_
            ? (int)std::floor(std::log2(std::max(frameW, frameH))) + 1
            : 1;

        glTextureStorage3D(texture_id_, levels, finalFmt, frameW, frameH, totalFrames);

        // slice the sprite sheet
        const uint8_t* fullData = sheet.GetDataU8();
        if (!fullData) {
            throw std::runtime_error("OpenGLTextureArray: expected 8-bit sprite sheet!");
        }
        size_t frameSize = (size_t)frameW * frameH * chans;
        std::vector<uint8_t> frameBuf(frameSize);

        uint32_t frameIdx = 0;
        for (uint32_t gy = 0; gy < gridY && frameIdx < totalFrames; ++gy) {
            for (uint32_t gx = 0; gx < gridX && frameIdx < totalFrames; ++gx) {
                uint32_t srcX = gx * frameW;
                uint32_t srcY = gy * frameH;
                for (uint32_t row = 0; row < frameH; ++row) {
                    size_t srcOffset = ((srcY + row) * (size_t)width_ + srcX) * chans;
                    size_t dstOffset = row * (size_t)frameW * chans;
                    std::memcpy(&frameBuf[dstOffset],
                        &fullData[srcOffset],
                        frameW * chans);
                }
                // upload subimage
                glTextureSubImage3D(texture_id_,
                    0, // mip
                    0, 0, frameIdx, // x,y,z
                    frameW, frameH, 1,
                    GL_RGBA, // assume 4 channels forced
                    GL_UNSIGNED_BYTE,
                    frameBuf.data());
                frameIdx++;
            }
        }
        Logger::GetLogger()->info("OpenGLTextureArray: Loaded {} frames (expected {}).", frameIdx, totalFrames);

        // set params
        if (config.generate_mips_) {
            glGenerateTextureMipmap(texture_id_);
        }
        glTextureParameteri(texture_id_, GL_TEXTURE_MIN_FILTER, config.min_filter_);
        glTextureParameteri(texture_id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_);
        glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_S, config.wrap_s_);
        glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_T, config.wrap_t_);
        glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_R, config.wrap_r_);

        if (config.use_anisotropy_) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            glTextureParameterf(texture_id_, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
        }

        MakeBindlessIfNeeded(config.use_bindless_);
    }

} // namespace graphics