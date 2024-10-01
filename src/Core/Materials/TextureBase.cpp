#include "TextureBase.h"

TextureBase::TextureBase()
    : m_RendererID(0), m_FilePath(""), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
}

TextureBase::TextureBase(const std::string& path)
	: m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
}

TextureBase::~TextureBase()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}
