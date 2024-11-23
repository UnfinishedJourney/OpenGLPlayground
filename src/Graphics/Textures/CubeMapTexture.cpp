#include "Graphics/Textures/CubeMapTexture.h"
#include <stb_image.h>
#include "Utilities/Logger.h"
#include <stdexcept>

CubeMapTexture::CubeMapTexture(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps)
    : TextureBase() {
    Logger::GetLogger()->info("CubeMapTexture initialized.");
    LoadCubeMap(facePaths, generateMipmaps);
}

void CubeMapTexture::LoadCubeMap(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps) {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, m_RendererIDPtr.get());
    if (*m_RendererIDPtr == 0) {
        Logger::GetLogger()->error("Failed to create OpenGL cube map texture.");
        throw std::runtime_error("Failed to create OpenGL cube map texture.");
    }

    int width = 0, height = 0, channels = 0;
    bool firstFace = true;

    for (size_t i = 0; i < facePaths.size(); ++i) {
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(facePaths[i].string().c_str(), &width, &height, &channels, 3); // Force RGB
        if (!data) {
            Logger::GetLogger()->error("Failed to load cube map face '{}'.", facePaths[i].string());
            throw std::runtime_error("Failed to load cube map face: " + facePaths[i].string());
        }

        if (firstFace) {
            glTextureStorage2D(*m_RendererIDPtr, 1, GL_RGB8, width, height);
            m_Width = width;
            m_Height = height;
            m_BPP = 3 * 8; // 3 channels, 8 bits each
            firstFace = false;
        }
        else {
            if (width != m_Width || height != m_Height) {
                stbi_image_free(data);
                Logger::GetLogger()->error("Cube map face '{}' has inconsistent dimensions.", facePaths[i].string());
                throw std::runtime_error("Inconsistent cube map face dimensions.");
            }
        }

        glTextureSubImage3D(*m_RendererIDPtr, 0, 0, 0, static_cast<GLint>(i), width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTextureParameteri(*m_RendererIDPtr, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(*m_RendererIDPtr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(*m_RendererIDPtr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*m_RendererIDPtr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*m_RendererIDPtr, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (generateMipmaps) {
        glGenerateTextureMipmap(*m_RendererIDPtr);
    }

    Logger::GetLogger()->info("Loaded CubeMapTexture with Renderer ID {}", *m_RendererIDPtr);
}

void CubeMapTexture::Bind(GLuint slot) const {
    glBindTextureUnit(slot, *m_RendererIDPtr);
    Logger::GetLogger()->debug("Bound CubeMapTexture ID {} to slot {}", *m_RendererIDPtr, slot);
}

void CubeMapTexture::Unbind(GLuint slot) const {
    glBindTextureUnit(slot, 0);
    Logger::GetLogger()->debug("Unbound CubeMapTexture from slot {}", slot);
}