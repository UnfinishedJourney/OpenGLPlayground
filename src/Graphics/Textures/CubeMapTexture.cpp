//#include "Graphics/Textures/TextureSkyBox.h"
//
//TextureSkyBox::TextureSkyBox(const std::string& path, const std::string& extension)
//    : TextureBase()
//{
//    GLint supported;
//    glGetInternalformativ(GL_TEXTURE_CUBE_MAP, GL_RGBA8, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
//    if (supported == GL_FALSE) {
//        std::cerr << "GL_RGBA8 format not supported for GL_TEXTURE_CUBE_MAP." << std::endl;
//    }
//
//    m_FilePath = path;
//    GLCall(glGenTextures(1, &m_RendererID));
//    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
//
//    const char* suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
//
//    std::string texName = path + "_" + suffixes[0] + extension;
//    GLubyte* data = stbi_load(texName.c_str(), &m_Width, &m_Height, &m_BPP, 3);
//    GLCall(glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, m_Width, m_Height));
//    GLCall(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data));
//
//    for (unsigned int i = 1; i < 6; i++)
//    {
//        texName = path + "_" + suffixes[i] + extension;
//        data = stbi_load(texName.c_str(), &m_Width, &m_Height, &m_BPP, 3);
//        if (data)
//        {
//            /*glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//                0, GL_RGB, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
//            );*/
//
//            GLCall(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data));
//            stbi_image_free(data);
//        }
//        else
//        {
//            std::cout << "Cubemap tex failed to load at path: " << i << std::endl;
//            stbi_image_free(data);
//        }
//        /*GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));*/
//    }
//
//    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
//    //GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
//    //GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0));
//
//}
//
//void TextureSkyBox::Bind(unsigned int slot) const
//{
//    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
//    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
//}
//
//void TextureSkyBox::Unbind() const
//{
//    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
//}
//
//GLuint SkyBoxHelper::LoadCubeMap(const std::string& baseName, const std::string& extension)
//{
//    GLuint texID;
//    glGenTextures(1, &texID);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
//
//    const char* suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
//    GLint w, h;
//
//    // Load the first one to get width/height
//    std::string texName = baseName + "_" + suffixes[0] + extension;
//    GLubyte* data = LoadPixels(texName, w, h, false);
//
//    // Allocate immutable storage for the whole cube map texture
//    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, w, h);
//    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
//    stbi_image_free(data);
//
//    // Load the other 5 cube-map faces
//    for (int i = 1; i < 6; i++) {
//        std::string texName = baseName + "_" + suffixes[i] + extension;
//        data = LoadPixels(texName, w, h, false);
//        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
//        stbi_image_free(data);
//    }
//
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//    return texID;
//}
//
//unsigned char* SkyBoxHelper::LoadPixels(const std::string& fName, int& width, int& height, bool flip)
//{
//    int bytesPerPix;
//    stbi_set_flip_vertically_on_load(flip);
//    unsigned char* data = stbi_load(fName.c_str(), &width, &height, &bytesPerPix, 4);
//    return data;
//}



#include "Graphics/Textures/CubeMapTexture.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <cmath>

// Define STB_IMAGE_IMPLEMENTATION in one source file
#include "stb_image.h"

// Constructor for six separate image paths
CubeMapTexture::CubeMapTexture(const std::array<std::string, 6>& facePaths, bool generateMipmaps, bool flipVertically)
    : TextureBase(), m_VerticalCrossPath(""), m_Extension("")
{
    // Set image flipping based on parameter
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

    // Generate OpenGL texture ID
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

    // Load each face
    for (unsigned int i = 0; i < facePaths.size(); ++i)
    {
        if (!LoadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, facePaths[i], flipVertically))
        {
            std::cerr << "Failed to load cube map face: " << facePaths[i] << std::endl;
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps if required
    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    // Unbind texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// Constructor for vertical cross image
CubeMapTexture::CubeMapTexture(const std::string& verticalCrossPath, const std::string& extension, bool generateMipmaps, bool flipVertically)
    : TextureBase(verticalCrossPath), m_VerticalCrossPath(verticalCrossPath), m_Extension(extension)
{
    // Set image flipping based on parameter
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

    // Load vertical cross image
    int width, height, channels;
    float* data = stbi_loadf(verticalCrossPath.c_str(), &width, &height, &channels, 3); // Force 3 channels (RGB)
    if (!data)
    {
        std::cerr << "Failed to load vertical cross image: " << verticalCrossPath << std::endl;
        return;
    }

    // Convert vertical cross to six cube faces
    std::array<std::vector<unsigned char>, 6> faceData = ConvertVerticalCrossToFaces(reinterpret_cast<unsigned char*>(data), width, height, 3);
    stbi_image_free(data);

    // Generate OpenGL texture ID
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

    // Define cube map faces order
    GLenum facesOrder[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, // Right
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // Left
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // Top
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // Bottom
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // Front
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // Back
    };

    // Load each face
    for (unsigned int i = 0; i < 6; ++i)
    {
        if (!faceData[i].empty())
        {
            glTexImage2D(facesOrder[i], 0, GL_RGB32F, width / 4, height / 3, 0, GL_RGB, GL_FLOAT, faceData[i].data());
        }
        else
        {
            std::cerr << "Face " << i << " data is empty for vertical cross: " << verticalCrossPath << std::endl;
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps if required
    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    // Unbind texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeMapTexture::~CubeMapTexture()
{
    // TextureBase destructor handles deletion
}

// Load a single face of the cube map
bool CubeMapTexture::LoadFace(GLenum face, const std::string& path, bool flipVertically)
{
    // Set image flipping based on parameter
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 3); // Force 3 channels (RGB)
    if (data)
    {
        glTexImage2D(face, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        return true;
    }
    else
    {
        std::cerr << "CubeMapTexture::LoadFace - Failed to load image: " << path << std::endl;
        return false;
    }
}

// Convert vertical cross image to six cube faces
std::array<std::vector<unsigned char>, 6> CubeMapTexture::ConvertVerticalCrossToFaces(const unsigned char* data, int width, int height, int channels) const
{
    // Common vertical cross layout:
    //       +----+
    //       | Top|
    // +----+----+----+----+
    // | Left| Front| Right| Back|
    // +----+----+----+----+
    //       | Bottom|
    //       +----+

    std::array<std::vector<unsigned char>, 6> faces;

    // Calculate size of each face
    int faceWidth = width / 4;
    int faceHeight = height / 3;

    // Define the offsets for each face in the vertical cross
    struct Face {
        GLenum target;
        int xOffset;
        int yOffset;
    };

    std::array<Face, 6> faceLayouts = {
        // Order: Right, Left, Top, Bottom, Front, Back
        Face{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2 * faceWidth, faceHeight },    // Right
        Face{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0 * faceWidth, faceHeight },    // Left
        Face{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1 * faceWidth, 0 * faceHeight }, // Top
        Face{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 1 * faceWidth, 2 * faceHeight }, // Bottom
        Face{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 1 * faceWidth, faceHeight },    // Front
        Face{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 3 * faceWidth, faceHeight }     // Back
    };

    for (unsigned int i = 0; i < faceLayouts.size(); ++i)
    {
        const Face& face = faceLayouts[i];
        faces[i].reserve(faceWidth * faceHeight * channels);

        for (int y = 0; y < faceHeight; ++y)
        {
            for (int x = 0; x < faceWidth; ++x)
            {
                int srcX = face.xOffset + x;
                int srcY = face.yOffset + y;
                int srcIndex = (srcY * width + srcX) * channels + 0;

                for (int c = 0; c < channels; ++c)
                {
                    faces[i].push_back(data[srcIndex + c]);
                }
            }
        }
    }

    return faces;
}

// Bind the cube map texture to the specified texture unit
void CubeMapTexture::Bind(unsigned int slot) const
{
    if (slot > 31)
    {
        std::cerr << "CubeMapTexture::Bind - Texture unit " << slot << " is out of range (0-31)." << std::endl;
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
}

// Unbind the cube map texture from the specified texture unit
void CubeMapTexture::Unbind(unsigned int slot) const
{
    if (slot > 31)
    {
        std::cerr << "CubeMapTexture::Unbind - Texture unit " << slot << " is out of range (0-31)." << std::endl;
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}