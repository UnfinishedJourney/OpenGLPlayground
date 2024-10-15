#include "Graphics/Textures/TextureBase.h"
#include "Utilities/Logger.h"
#include "stb_image.h"

TextureBase::TextureBase()
    : m_RendererID(0), m_FilePath(""), m_LocalBuffer(nullptr),
    m_Width(0), m_Height(0), m_BPP(0)
{
}

TextureBase::TextureBase(const std::string& path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr),
    m_Width(0), m_Height(0), m_BPP(0)
{
}

TextureBase::~TextureBase()
{
    Release();
}

TextureBase::TextureBase(TextureBase&& other) noexcept
    : m_RendererID(other.m_RendererID),
    m_FilePath(std::move(other.m_FilePath)),
    m_LocalBuffer(other.m_LocalBuffer),
    m_Width(other.m_Width),
    m_Height(other.m_Height),
    m_BPP(other.m_BPP)
{
    other.m_RendererID = 0;
    other.m_LocalBuffer = nullptr;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_BPP = 0;
    Logger::GetLogger()->info("Moved TextureBase. New ID: {}.", m_RendererID);
}

TextureBase& TextureBase::operator=(TextureBase&& other) noexcept
{
    if (this != &other)
    {
        Release();

        m_RendererID = other.m_RendererID;
        m_FilePath = std::move(other.m_FilePath);
        m_LocalBuffer = other.m_LocalBuffer;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_BPP = other.m_BPP;

        other.m_RendererID = 0;
        other.m_LocalBuffer = nullptr;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_BPP = 0;

        Logger::GetLogger()->info("Assigned TextureBase. New ID: {}.", m_RendererID);
    }
    return *this;
}

void TextureBase::Release() noexcept
{
    if (m_RendererID != 0)
    {
        Logger::GetLogger()->info("Deleting TextureBase with ID {}.", m_RendererID);
        GLCall(glDeleteTextures(1, &m_RendererID));
        m_RendererID = 0;
    }
    if (m_LocalBuffer)
    {
        stbi_image_free(m_LocalBuffer);
        m_LocalBuffer = nullptr;
        Logger::GetLogger()->info("Freed local buffer memory for TextureBase.");
    }
}