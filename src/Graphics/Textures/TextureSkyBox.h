#pragma once

#include "Graphics/Textures/TextureBase.h"
#include "Utilities/Utility.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <iostream>

class TextureSkyBox : public TextureBase
{

public:
	TextureSkyBox(const std::string& path, const std::string& extension);
	virtual ~TextureSkyBox() = default;
	virtual void Bind(unsigned int slot = 0) const override;
	virtual void Unbind() const override;

};

class SkyBoxHelper
{
public:
	GLuint LoadCubeMap(const std::string& baseName, const std::string& extension);
private:
	unsigned char* LoadPixels(const std::string& fName, int& width, int& height, bool flip);
};