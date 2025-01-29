#include "OpenGLCubeMapTexture.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <cmath>

static GLenum ResolveCubeMapFormat(const TextureConfig& config) {
    // If user wants HDR, default to RGBA16F, else sRGB or RGBA8
    if (config.isHDR) {
        return (config.internalFormat == GL_RGBA8 || config.internalFormat == GL_SRGB8_ALPHA8)
            ? GL_RGBA16F
            : config.internalFormat;
    }
    else if (config.isSRGB) {
        return (config.internalFormat == GL_RGBA8)
            ? GL_SRGB8_ALPHA8
            : config.internalFormat;
    }
    return config.internalFormat;
}

OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
    TextureConfig config)
{
    // Load each face
    std::array<TextureData, 6> faceData;
    for (size_t i = 0; i < 6; i++) {
        // Decide if it's HDR or not. If you want to allow mixing, you'd need more logic.
        bool faceIsHDR = config.isHDR;
        if (!faceData[i].LoadFromFile(faces[i].string(), false, true, faceIsHDR)) {
            throw std::runtime_error("Failed to load cube face: " + faces[i].string());
        }
    }

    // Validate consistent sizes
    m_Width = faceData[0].GetWidth();
    m_Height = faceData[0].GetHeight();
    for (int i = 1; i < 6; i++) {
        if (faceData[i].GetWidth() != m_Width ||
            faceData[i].GetHeight() != m_Height)
        {
            throw std::runtime_error("All cube faces must have same dimensions.");
        }
    }

    // Choose internal format
    config.internalFormat = ResolveCubeMapFormat(config);

    GLCall(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID));
    if (!m_TextureID) {
        throw std::runtime_error("Failed to create cube map texture object.");
    }

    int levels = config.generateMips
        ? static_cast<int>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1
        : 1;

    GLCall(glTextureStorage2D(m_TextureID, levels, config.internalFormat, m_Width, m_Height));

    // Upload each face
    for (int i = 0; i < 6; i++) {
        GLenum dataType = faceData[i].IsHDR() ? GL_FLOAT : GL_UNSIGNED_BYTE;
        const void* dataPtr = faceData[i].IsHDR()
            ? static_cast<const void*>(faceData[i].GetDataFloat())
            : static_cast<const void*>(faceData[i].GetDataU8());

        // Typically we assume RGBA for the subimage if forced 4Ch
        GLenum uploadFormat = GL_RGBA;

        GLCall(glTextureSubImage3D(
            m_TextureID,
            0,
            0, 0, i,    // x, y, slice=cubeFaceIndex
            m_Width,
            m_Height,
            1,         // depth=1 for each face
            uploadFormat,
            dataType,
            dataPtr
        ));
    }

    if (config.generateMips) {
        GLCall(glGenerateTextureMipmap(m_TextureID));
    }

    // Wrap / filter
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));
    GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR));

    if (config.useAnisotropy) {
        GLfloat maxAniso = 0.0f;
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

    Logger::GetLogger()->info("Created OpenGLCubeMapTexture (ID={}) {}x{}; HDR={}, sRGB={}.",
        m_TextureID, m_Width, m_Height, config.isHDR, config.isSRGB);
}

OpenGLCubeMapTexture::~OpenGLCubeMapTexture()
{
    if (m_IsBindless && m_BindlessHandle) {
        glMakeTextureHandleNonResidentARB(m_BindlessHandle);
    }
    if (m_TextureID) {
        GLCall(glDeleteTextures(1, &m_TextureID));
    }
}

void OpenGLCubeMapTexture::Bind(uint32_t unit) const
{
    if (!m_IsBindless) {
        GLCall(glBindTextureUnit(unit, m_TextureID));
    }
}

void OpenGLCubeMapTexture::Unbind(uint32_t unit) const
{
    if (!m_IsBindless) {
        GLCall(glBindTextureUnit(unit, 0));
    }
}