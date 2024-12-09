#pragma once

#include "Graphics/Textures/TextureBase.h"
#include <string_view>
#include <filesystem>

class Texture2D : public TextureBase {
public:
    Texture2D() = default;

    Texture2D(GLuint textureID, int width, int height, GLenum internalFormat)
        : TextureBase()
    {
        if (textureID == 0) {
            throw std::invalid_argument("Invalid texture ID provided to Texture2D constructor.");
        }

        *m_RendererIDPtr = textureID;

        m_Width = width;
        m_Height = height;
        m_BPP = (internalFormat == GL_RG32F) ? 8 : 32; 
        m_FilePath = "Procedurally Generated BRDF LUT";

        Logger::GetLogger()->info("Texture2D wrapped with existing Renderer ID: {}", *m_RendererIDPtr);
    }

    explicit Texture2D(std::filesystem::path filePath);

    Texture2D(Texture2D&& other) noexcept = default;
    Texture2D& operator=(Texture2D&& other) noexcept = default;

    Texture2D(const Texture2D& other) = delete;
    Texture2D& operator=(const Texture2D& other) = delete;

    void Bind(GLuint slot = 0) const override;
    void Unbind(GLuint slot = 0) const override;

private:
    void LoadTexture();
};