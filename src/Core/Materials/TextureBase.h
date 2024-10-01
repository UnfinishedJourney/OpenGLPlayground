#pragma once

#include "Utility.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <iostream>

class TextureBase
{
public:
	TextureBase();
	TextureBase(const std::string& path);
	virtual ~TextureBase();
	virtual void Bind(unsigned int slot = 0) const = 0;
	virtual void Unbind() const = 0;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

protected:
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
};