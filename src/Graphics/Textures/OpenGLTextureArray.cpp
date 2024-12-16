#include "OpenGLTextureArray.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <cmath>

OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& filePaths, const TextureConfig& config,
    uint32_t totalFrames, uint32_t gridX, uint32_t gridY) {
    if (filePaths.empty()) {
        throw std::runtime_error("No files for texture array.");
    }

    if (filePaths.size() > 1) {
        throw std::runtime_error("Multiple files not supported in this scenario.");
    }

    TextureData data;
    if (!data.LoadFromFile(filePaths[0], false)) { 
        throw std::runtime_error("Failed to load texture: " + filePaths[0]);
    }

    m_Width = data.GetWidth();
    m_Height = data.GetHeight();

    uint32_t frameWidth = m_Width / gridX;
    uint32_t frameHeight = m_Height / gridY;
    //if (frameWidth * gridX != m_Width || frameHeight * gridY != m_Height) {
    //    throw std::runtime_error("Sprite sheet dimensions not divisible by grid.");
    //}

    GLCall(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_TextureID));
    if (!m_TextureID) throw std::runtime_error("Failed to create texture array.");

    int levels = config.generateMips ? (int)std::floor(std::log2(std::max(frameWidth, frameHeight))) + 1 : 1;
    GLCall(glTextureStorage3D(m_TextureID, levels, config.internalFormat, frameWidth, frameHeight, totalFrames));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const unsigned char* fullImageData = data.GetData();
    int channels = data.GetChannels(); 

    std::vector<unsigned char> frameData(frameWidth * frameHeight * channels);

    uint32_t frameIndex = 0;
    for (uint32_t y = 0; y < gridY && frameIndex < totalFrames; y++) {
        for (uint32_t x = 0; x < gridX && frameIndex < totalFrames; x++) {
            uint32_t srcX = x * frameWidth;
            uint32_t srcY = y * frameHeight;

            for (uint32_t row = 0; row < frameHeight; row++) {
                const unsigned char* srcPtr = fullImageData + ((srcY + row) * m_Width + srcX) * channels;
                unsigned char* dstPtr = frameData.data() + (row * frameWidth) * channels;
                std::memcpy(dstPtr, srcPtr, frameWidth * channels);
            }

            GLCall(glTextureSubImage3D(m_TextureID, 0, 0, 0, frameIndex,
                frameWidth, frameHeight, 1,
                GL_RGBA, GL_UNSIGNED_BYTE, frameData.data()));

            frameIndex++;
        }
    }

    Logger::GetLogger()->info("OpenGLTextureArray: Loaded {} frames into texture array (expected {}).", frameIndex, totalFrames);

    if (config.generateMips) GLCall(glGenerateTextureMipmap(m_TextureID));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR));

    if (config.useAnisotropy && GLAD_GL_EXT_texture_filter_anisotropic) {
        GLfloat maxAniso;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        GLCall(glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso));
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
    if (m_IsBindless && m_BindlessHandle) glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
}

void OpenGLTextureArray::Bind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, m_TextureID);
}

void OpenGLTextureArray::Unbind(uint32_t unit) const {
    if (!m_IsBindless) glBindTextureUnit(unit, 0);
}