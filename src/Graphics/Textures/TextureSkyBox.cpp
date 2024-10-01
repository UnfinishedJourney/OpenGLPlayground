#include "Graphics/Textures/TextureSkyBox.h"

TextureSkyBox::TextureSkyBox(const std::string& path, const std::string& extension)
    : TextureBase()
{
    GLint supported;
    glGetInternalformativ(GL_TEXTURE_CUBE_MAP, GL_RGBA8, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
    if (supported == GL_FALSE) {
        std::cerr << "GL_RGBA8 format not supported for GL_TEXTURE_CUBE_MAP." << std::endl;
    }

    m_FilePath = path;
    GLCall(glGenTextures(1, &m_RendererID));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));

    const char* suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

    std::string texName = path + "_" + suffixes[0] + extension;
    GLubyte* data = stbi_load(texName.c_str(), &m_Width, &m_Height, &m_BPP, 3);
    GLCall(glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, m_Width, m_Height));
    GLCall(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data));

    for (unsigned int i = 1; i < 6; i++)
    {
        texName = path + "_" + suffixes[i] + extension;
        data = stbi_load(texName.c_str(), &m_Width, &m_Height, &m_BPP, 3);
        if (data)
        {
            /*glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );*/

            GLCall(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data));
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << i << std::endl;
            stbi_image_free(data);
        }
        /*GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));*/
    }

    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    //GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
    //GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0));

}

void TextureSkyBox::Bind(unsigned int slot) const
{
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
}

void TextureSkyBox::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

GLuint SkyBoxHelper::LoadCubeMap(const std::string& baseName, const std::string& extension)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    const char* suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
    GLint w, h;

    // Load the first one to get width/height
    std::string texName = baseName + "_" + suffixes[0] + extension;
    GLubyte* data = LoadPixels(texName, w, h, false);

    // Allocate immutable storage for the whole cube map texture
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, w, h);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    // Load the other 5 cube-map faces
    for (int i = 1; i < 6; i++) {
        std::string texName = baseName + "_" + suffixes[i] + extension;
        data = LoadPixels(texName, w, h, false);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texID;
}

unsigned char* SkyBoxHelper::LoadPixels(const std::string& fName, int& width, int& height, bool flip)
{
    int bytesPerPix;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(fName.c_str(), &width, &height, &bytesPerPix, 4);
    return data;
}