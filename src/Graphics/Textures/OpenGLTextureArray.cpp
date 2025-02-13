#include "OpenGLTextureArray.h"
#include "TextureData.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace graphics {

    OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& filePaths,
        const TextureConfig& config,
        uint32_t totalFrames,
        uint32_t gridX,
        uint32_t gridY)
    {
        if (filePaths.empty()) {
            throw std::runtime_error("OpenGLTextureArray: No input files provided.");
        }
        if (filePaths.size() > 1) {
            // This sample shows slicing a single sprite sheet.
            throw std::runtime_error("OpenGLTextureArray: Multiple files not supported in this scenario.");
        }

        // Load the full image from the first file.
        TextureData data;
        if (!data.LoadFromFile(filePaths[0], /*flipY=*/false, /*force4Ch=*/true, /*isHDR=*/false)) {
            throw std::runtime_error("OpenGLTextureArray: Failed to load texture from " + filePaths[0]);
        }

        m_Width = static_cast<uint32_t>(data.GetWidth());
        m_Height = static_cast<uint32_t>(data.GetHeight());
        int channels = data.GetChannels();

        // Compute per-frame dimensions.
        uint32_t frameWidth = m_Width / gridX;
        uint32_t frameHeight = m_Height / gridY;

        GLCall(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_TextureID));
        if (!m_TextureID) {
            throw std::runtime_error("OpenGLTextureArray: Failed to create GL texture array object.");
        }

        int levels = config.generateMips
            ? static_cast<int>(std::floor(std::log2(std::max(frameWidth, frameHeight)))) + 1
            : 1;
        GLCall(glTextureStorage3D(m_TextureID, levels, config.internalFormat, frameWidth, frameHeight, totalFrames));

        // Prepare to slice the full image.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        const unsigned char* fullImageData = data.GetDataU8();
        if (!fullImageData) {
            throw std::runtime_error("OpenGLTextureArray: No 8-bit data found (are you loading HDR?).");
        }

        size_t frameSize = static_cast<size_t>(frameWidth) * frameHeight * channels;
        std::vector<unsigned char> frameData(frameSize, 0);
        uint32_t frameIndex = 0;
        for (uint32_t gy = 0; gy < gridY && frameIndex < totalFrames; gy++) {
            for (uint32_t gx = 0; gx < gridX && frameIndex < totalFrames; gx++) {
                uint32_t srcX = gx * frameWidth;
                uint32_t srcY = gy * frameHeight;
                for (uint32_t row = 0; row < frameHeight; row++) {
                    size_t srcOffset = ((srcY + row) * static_cast<size_t>(m_Width) + srcX) * channels;
                    size_t dstOffset = row * static_cast<size_t>(frameWidth) * channels;
                    std::memcpy(&frameData[dstOffset], &fullImageData[srcOffset], frameWidth * channels);
                }
                GLCall(glTextureSubImage3D(m_TextureID,
                    0, // mip level
                    0, // x offset
                    0, // y offset
                    frameIndex, // layer index
                    frameWidth,
                    frameHeight,
                    1, // one layer at a time
                    GL_RGBA, // forced 4 channels
                    GL_UNSIGNED_BYTE,
                    frameData.data()));
                frameIndex++;
            }
        }
        Logger::GetLogger()->info("OpenGLTextureArray: Loaded {} frames (expected {}).", frameIndex, totalFrames);

        if (config.generateMips) {
            GLCall(glGenerateTextureMipmap(m_TextureID));
        }
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR));

        if (config.useAnisotropy) {
            GLfloat maxAniso = 0.f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            GLCall(glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso));
        }
        if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
            m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
            if (m_BindlessHandle) {
                glMakeTextureHandleResidentARB(m_BindlessHandle);
                m_IsBindless = true;
            }
        }
    }

    OpenGLTextureArray::~OpenGLTextureArray() {
        if (m_IsBindless && m_BindlessHandle) {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
        }
        if (m_TextureID) {
            GLCall(glDeleteTextures(1, &m_TextureID));
        }
    }

    void OpenGLTextureArray::Bind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, m_TextureID));
        }
    }

    void OpenGLTextureArray::Unbind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace graphics