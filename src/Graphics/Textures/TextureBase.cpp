#include "Graphics/Textures/TextureBase.h"
#include <stb_image.h>

TextureBase::TextureBase(std::filesystem::path filePath)
    : m_FilePath(std::move(filePath)) {
    Logger::GetLogger()->info("TextureBase initialized with file path: '{}'", m_FilePath.string());
}

TextureBase::~TextureBase() {
    Release();
}

TextureBase::TextureBase(TextureBase&& other) noexcept
    : m_RendererID(other.m_RendererID),
    m_FilePath(std::move(other.m_FilePath)),
    m_Width(other.m_Width),
    m_Height(other.m_Height),
    m_BPP(other.m_BPP) {
    other.m_RendererID = 0;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_BPP = 0;
    Logger::GetLogger()->info("Moved TextureBase. New Renderer ID: {}", m_RendererID);
}

TextureBase& TextureBase::operator=(TextureBase&& other) noexcept {
    if (this != &other) {
        Release();

        m_RendererID = other.m_RendererID;
        m_FilePath = std::move(other.m_FilePath);
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_BPP = other.m_BPP;

        other.m_RendererID = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_BPP = 0;

        Logger::GetLogger()->info("Moved-assigned TextureBase. New Renderer ID: {}", m_RendererID);
    }
    return *this;
}

void TextureBase::Release() noexcept {
    if (m_RendererID != 0) {
        glDeleteTextures(1, &m_RendererID);
        Logger::GetLogger()->info("Deleted TextureBase with Renderer ID {}", m_RendererID);
        m_RendererID = 0;
    }
}