#pragma once

#include "Utilities/Utility.h"
#include <glad/glad.h>
#include <string>
#include <iostream>
#include <stdexcept>

class TextureBase
{
public:
    TextureBase();
    explicit TextureBase(const std::string& path);
    virtual ~TextureBase();

    TextureBase(const TextureBase& other) = delete;
    TextureBase& operator=(const TextureBase& other) = delete;
    TextureBase(TextureBase&& other) noexcept;
    TextureBase& operator=(TextureBase&& other) noexcept;

    virtual void Bind(unsigned int slot = 0) const = 0;
    virtual void Unbind(unsigned int slot = 0) const = 0;

    inline int GetWidth() const { return m_Width; }
    inline int GetHeight() const { return m_Height; }
    inline GLuint GetRendererID() const { return m_RendererID; }
    inline const std::string& GetFilePath() const { return m_FilePath; }

protected:
    virtual void Release() noexcept;

    GLuint m_RendererID = 0;
    std::string m_FilePath;
    unsigned char* m_LocalBuffer = nullptr;
    int m_Width = 0, m_Height = 0, m_BPP = 0;

};