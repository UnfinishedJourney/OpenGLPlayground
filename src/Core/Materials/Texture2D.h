#pragma once

#include "TextureBase.h"
#include "Utility.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <iostream>

class Texture2D : public TextureBase
{
public:
	Texture2D();
	Texture2D(const std::string& path);
	virtual void Bind(unsigned int slot = 0) const override;
	virtual void Unbind() const override;
};