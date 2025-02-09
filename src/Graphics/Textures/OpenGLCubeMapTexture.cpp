#include "OpenGLCubeMapTexture.h"
#include "TextureData.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <vector>

namespace Graphics {

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
        const TextureConfig& config)
    {
        // Load the first face to determine dimensions.
        TextureData faceData;
        if (!faceData.LoadFromFile(faces[0].string(), /*flipY=*/false, /*force4Ch=*/true, config.isHDR)) {
            throw std::runtime_error("Failed to load cube map face: " + faces[0].string());
        }
        m_Width = static_cast<uint32_t>(faceData.GetWidth());
        m_Height = static_cast<uint32_t>(faceData.GetHeight());
        GLenum internalFormat = config.internalFormat;

        GLCall(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID));
        if (!m_TextureID) {
            throw std::runtime_error("Failed to create cube map texture.");
        }

        int levels = config.generateMips
            ? static_cast<int>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1
            : 1;
        GLCall(glTextureStorage2D(m_TextureID, levels, internalFormat, m_Width, m_Height));

        // Load and upload each face.
        for (size_t i = 0; i < 6; i++) {
            TextureData data;
            if (!data.LoadFromFile(faces[i].string(), /*flipY=*/false, /*force4Ch=*/true, config.isHDR)) {
                throw std::runtime_error("Failed to load cube map face: " + faces[i].string());
            }
            GLenum dataType = config.isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
            GLenum uploadFormat = GL_RGBA;
            const void* pixels = config.isHDR
                ? static_cast<const void*>(data.GetDataFloat())
                : static_cast<const void*>(data.GetDataU8());
            if (!pixels) {
                throw std::runtime_error("No pixel data for cube map face: " + faces[i].string());
            }
            GLCall(glTextureSubImage3D(m_TextureID, 0, 0, 0, i, m_Width, m_Height, 1, uploadFormat, dataType, pixels));
        }
        if (config.generateMips) {
            GLCall(glGenerateTextureMipmap(m_TextureID));
        }
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR));

        if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
            m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
            if (m_BindlessHandle) {
                glMakeTextureHandleResidentARB(m_BindlessHandle);
                m_IsBindless = true;
            }
        }
    }

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mipFaces,
        const TextureConfig& config)
    {
        if (mipFaces.empty()) {
            throw std::runtime_error("No mip faces provided for cube map texture.");
        }
        // Use the first mip level to determine dimensions.
        TextureData faceData;
        if (!faceData.LoadFromFile(mipFaces[0][0].string(), false, true, config.isHDR)) {
            throw std::runtime_error("Failed to load cube map face: " + mipFaces[0][0].string());
        }
        m_Width = static_cast<uint32_t>(faceData.GetWidth());
        m_Height = static_cast<uint32_t>(faceData.GetHeight());
        GLenum internalFormat = config.internalFormat;

        int mipLevels = static_cast<int>(mipFaces.size());
        GLCall(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID));
        if (!m_TextureID) {
            throw std::runtime_error("Failed to create cube map texture.");
        }
        GLCall(glTextureStorage2D(m_TextureID, mipLevels, internalFormat, m_Width, m_Height));

        for (int level = 0; level < mipLevels; ++level) {
            const auto& faces = mipFaces[level];
            for (size_t i = 0; i < 6; i++) {
                TextureData data;
                if (!data.LoadFromFile(faces[i].string(), false, true, config.isHDR)) {
                    throw std::runtime_error("Failed to load cube map face at mip level " + std::to_string(level) +
                        ": " + faces[i].string());
                }
                GLenum dataType = config.isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
                GLenum uploadFormat = GL_RGBA;
                const void* pixels = config.isHDR
                    ? static_cast<const void*>(data.GetDataFloat())
                    : static_cast<const void*>(data.GetDataU8());
                if (!pixels) {
                    throw std::runtime_error("No pixel data for cube map face at mip level " + std::to_string(level) +
                        ": " + faces[i].string());
                }
                GLCall(glTextureSubImage3D(m_TextureID, level, 0, 0, i, m_Width, m_Height, 1, uploadFormat, dataType, pixels));
            }
        }
        if (config.generateMips) {
            GLCall(glGenerateTextureMipmap(m_TextureID));
        }
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, config.minFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, config.magFilter));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, config.wrapS));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, config.wrapT));
        GLCall(glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, config.wrapR));

        if (config.useBindless && GLAD_GL_ARB_bindless_texture) {
            m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
            if (m_BindlessHandle) {
                glMakeTextureHandleResidentARB(m_BindlessHandle);
                m_IsBindless = true;
            }
        }
    }

    OpenGLCubeMapTexture::~OpenGLCubeMapTexture() {
        if (m_IsBindless && m_BindlessHandle) {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
        }
        if (m_TextureID) {
            GLCall(glDeleteTextures(1, &m_TextureID));
        }
    }

    void OpenGLCubeMapTexture::Bind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, m_TextureID));
        }
    }

    void OpenGLCubeMapTexture::Unbind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace Graphics