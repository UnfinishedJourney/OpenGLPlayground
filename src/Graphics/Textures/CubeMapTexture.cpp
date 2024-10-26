#include "Graphics/Textures/CubeMapTexture.h"
#include <stb_image.h>
#include "Utilities/Logger.h"

CubeMapTexture::CubeMapTexture(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps)
    : TextureBase() {
    LoadCubeMap(facePaths, generateMipmaps);
}

void CubeMapTexture::LoadCubeMap(const std::array<std::filesystem::path, 6>& facePaths, bool generateMipmaps) {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create OpenGL cube map texture.");
        throw std::runtime_error("Failed to create OpenGL cube map texture.");
    }

    // Assuming all faces have the same dimensions
    int width = 0, height = 0, channels = 0;

    for (size_t i = 0; i < facePaths.size(); ++i) {
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(facePaths[i].string().c_str(), &width, &height, &channels, 3);
        if (!data) {
            Logger::GetLogger()->error("Failed to load cube map face '{}'.", facePaths[i].string());
            throw std::runtime_error("Failed to load cube map face: " + facePaths[i].string());
        }

        if (i == 0) {
            // Allocate immutable storage for the cube map
            glTextureStorage2D(m_RendererID, 1, GL_RGB8, width, height);
        }

        glTextureSubImage3D(m_RendererID, 0, 0, 0, static_cast<GLint>(i), width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (generateMipmaps) {
        glGenerateTextureMipmap(m_RendererID);
    }

    Logger::GetLogger()->info("Loaded CubeMapTexture with Renderer ID {}", m_RendererID);
}

void CubeMapTexture::Bind(GLuint slot) const {
    glBindTextureUnit(slot, m_RendererID);
    Logger::GetLogger()->debug("Bound CubeMapTexture ID {} to slot {}", m_RendererID, slot);
}

void CubeMapTexture::Unbind(GLuint slot) const {
    glBindTextureUnit(slot, 0);
    Logger::GetLogger()->debug("Unbound CubeMapTexture from slot {}", slot);
}