#pragma once

#include "Utility.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <iostream>
class Texture
{
public:
	Texture();
	Texture(const std::string& path);
	virtual ~Texture();
	virtual void Bind(unsigned int slot = 0) const;
	virtual void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

protected:
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
};

class SkyBoxTexture : public Texture
{

public:
	SkyBoxTexture(const std::string& path, const std::string& extension);
	virtual ~SkyBoxTexture() = default;
	virtual void Bind(unsigned int slot = 0) const;
	virtual void Unbind() const;

};

class SkyBoxHelper
{
public:
	GLuint LoadCubeMap(const std::string& baseName, const std::string& extension);
private:
	unsigned char* LoadPixels(const std::string& fName, int& width, int& height, bool flip);
};