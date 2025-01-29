#include "OpenGLTextureArray.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"

#include <cmath>
#include <cstring>    // for std::memcpy
#include <stdexcept>

/**
 * @brief Constructor: loads a single file (sprite sheet), slices it into totalFrames
 *        based on gridX * gridY, and stores them in a 2D texture array.
 */
OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& filePaths,
    const TextureConfig& config,
    uint32_t totalFrames,
    uint32_t gridX,
    uint32_t gridY)
    : m_TextureIDPtr(std::make_unique<GLuint>(0).release(), TextureDeleter())
{
    if (filePaths.empty()) {
        throw std::runtime_error("OpenGLTextureArray: No input files provided.");
    }
    if (filePaths.size() > 1) {
        // This sample only shows how to slice a single file. 
        // If you do want multiple files => each becomes a layer.
        throw std::runtime_error("OpenGLTextureArray: Multiple files not supported in this scenario.");
    }

    // 1) Load image data from the first (and only) file
    TextureData data;
    // For now, we assume 8-bit LDR, no flipping (or up to you)
    if (!data.LoadFromFile(filePaths[0], /*flipY*/false, /*force4Ch*/true, /*isHDR*/false)) {
        throw std::runtime_error("OpenGLTextureArray: Failed to load texture from " + filePaths[0]);
    }

    m_Width = static_cast<uint32_t>(data.GetWidth());
    m_Height = static_cast<uint32_t>(data.GetHeight());
    int channels = data.GetChannels(); // e.g. 4 if forced RGBA

    // 2) Compute per-frame sub-image sizes from the sprite sheet
    uint32_t frameWidth = m_Width / gridX;
    uint32_t frameHeight = m_Height / gridY;

    // Optionally check if the sheet divides evenly
    // if (frameWidth * gridX != m_Width || frameHeight * gridY != m_Height) {
    //     throw std::runtime_error("Sprite sheet not evenly divisible by gridX/gridY.");
    // }

    // 3) Create the GL texture object
    GLCall(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, m_TextureIDPtr.get()));
    if (*m_TextureIDPtr == 0) {
        throw std::runtime_error("OpenGLTextureArray: Failed to create GL texture array object.");
    }

    // 4) Allocate storage
    // Compute number of mip levels if needed
    int levels = config.generateMips
        ? static_cast<int>(std::floor(std::log2(std::max(frameWidth, frameHeight)))) + 1
        : 1;

    GLCall(glTextureStorage3D(
        *m_TextureIDPtr,
        levels,
        config.internalFormat,
        frameWidth,       // each layer's width
        frameHeight,      // each layer's height
        totalFrames       // number of layers
    ));

    // 5) Prepare to upload sub-images
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const unsigned char* fullImageData = data.GetDataU8();
    if (!fullImageData) {
        throw std::runtime_error("OpenGLTextureArray: No 8-bit data found (are you loading HDR?).");
    }

    // Temporary buffer for one frame's worth of pixels
    size_t frameSize = static_cast<size_t>(frameWidth) * frameHeight * channels;
    std::vector<unsigned char> frameData(frameSize, 0);

    // 6) Slice each frame out of the sprite sheet and upload
    uint32_t frameIndex = 0;
    for (uint32_t gy = 0; gy < gridY && frameIndex < totalFrames; gy++) {
        for (uint32_t gx = 0; gx < gridX && frameIndex < totalFrames; gx++) {
            uint32_t srcX = gx * frameWidth;
            uint32_t srcY = gy * frameHeight;

            // Copy row-by-row into frameData
            for (uint32_t row = 0; row < frameHeight; row++) {
                const size_t srcOffset =
                    ((srcY + row) * static_cast<size_t>(m_Width) + srcX) * channels;

                const size_t dstOffset = row * static_cast<size_t>(frameWidth) * channels;

                std::memcpy(
                    &frameData[dstOffset],
                    &fullImageData[srcOffset],
                    frameWidth * channels
                );
            }

            // Upload this frame as one layer in the array
            GLCall(glTextureSubImage3D(
                *m_TextureIDPtr,
                0, // mip level
                0, // x offset
                0, // y offset
                frameIndex, // z offset => layer index
                frameWidth,
                frameHeight,
                1, // depth=1 => just one layer
                GL_RGBA,            // we forced 4 channels => RGBA
                GL_UNSIGNED_BYTE,   // LDR => 8-bit
                frameData.data()
            ));

            frameIndex++;
        }
    }

    Logger::GetLogger()->info(
        "OpenGLTextureArray: Loaded {} frames (expected {}).",
        frameIndex, totalFrames
    );

    // 7) Generate MIP maps if requested
    if (config.generateMips) {
        GLCall(glGenerateTextureMipmap(*m_TextureIDPtr));
    }

    // 8) Set texture parameters
    GLCall(glTextureParameteri(*m_TextureIDPtr, GL_TEXTURE_MIN_FILTER, config.minFilter));
    GLCall(glTextureParameteri(*m_TextureIDPtr, GL_TEXTURE_MAG_FILTER, config.magFilter));
    GLCall(glTextureParameteri(*m_TextureIDPtr, GL_TEXTURE_WRAP_S, config.wrapS));
    GLCall(glTextureParameteri(*m_TextureIDPtr, GL_TEXTURE_WRAP_T, config.wrapT));
    GLCall(glTextureParameteri(*m_TextureIDPtr, GL_TEXTURE_WRAP_R, config.wrapR));

    // Optional anisotropy
    if (config.useAnisotropy) {
        GLfloat maxAniso = 0.f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        GLCall(glTextureParameterf(*m_TextureIDPtr, GL_TEXTURE_MAX_ANISOTROPY, maxAniso));
    }

    // 9) Bindless if requested
    if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
        m_BindlessHandle = glGetTextureHandleARB(*m_TextureIDPtr);
        if (m_BindlessHandle) {
            glMakeTextureHandleResidentARB(m_BindlessHandle);
            m_IsBindless = true;
        }
    }
}

OpenGLTextureArray::~OpenGLTextureArray()
{
    // If bindless, revoke the residency
    if (m_IsBindless && m_BindlessHandle) {
        glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    }
    // The actual glDeleteTextures call will happen via m_TextureIDPtr + TextureDeleter
}

void OpenGLTextureArray::Bind(uint32_t unit) const
{
    // If bindless is used, typically the shader just uses the handle.
    // But we implement a fallback bind for non-bindless usage:
    if (!m_IsBindless) {
        glBindTextureUnit(unit, *m_TextureIDPtr);
    }
}

void OpenGLTextureArray::Unbind(uint32_t unit) const
{
    if (!m_IsBindless) {
        glBindTextureUnit(unit, 0);
    }
}