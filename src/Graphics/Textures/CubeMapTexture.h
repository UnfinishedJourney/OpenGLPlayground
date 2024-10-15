//#pragma once
//
//#include "Graphics/Textures/TextureBase.h"
//#include "Utilities/Utility.h"
//#include "stb_image.h"
//#include <glad/glad.h>
//#include <string>
//#include <iostream>
//
//class TextureSkyBox : public TextureBase
//{
//
//public:
//	TextureSkyBox(const std::string& path, const std::string& extension);
//	virtual ~TextureSkyBox() = default;
//	virtual void Bind(unsigned int slot = 0) const override;
//	virtual void Unbind() const override;
//
//};
//
//class SkyBoxHelper
//{
//public:
//	GLuint LoadCubeMap(const std::string& baseName, const std::string& extension);
//private:
//	unsigned char* LoadPixels(const std::string& fName, int& width, int& height, bool flip);
//};


#pragma once

#include "Graphics/Textures/TextureBase.h"
#include <vector>
#include <string>
#include <array>

class CubeMapTexture : public TextureBase
{
public:
    // Constructor for loading from six separate image paths
    CubeMapTexture(const std::array<std::string, 6>& facePaths, bool generateMipmaps = true, bool flipVertically = false);

    // Constructor for loading from a vertical cross image
    CubeMapTexture(const std::string& verticalCrossPath, const std::string& extension, bool generateMipmaps = true, bool flipVertically = false);

    virtual ~CubeMapTexture();

    virtual void Bind(unsigned int slot = 0) const override;
    virtual void Unbind(unsigned int slot = 0) const override;

private:
    // Helper function to load a single face of the cube map
    bool LoadFace(GLenum face, const std::string& path, bool flipVertically);

    // Helper function to convert vertical cross to six cube faces
    std::array<std::vector<unsigned char>, 6> ConvertVerticalCrossToFaces(const unsigned char* data, int width, int height, int channels) const;

    // Path to the vertical cross image
    std::string m_VerticalCrossPath;

    // Extension of the images
    std::string m_Extension;
};