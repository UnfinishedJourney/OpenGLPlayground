#pragma once

#include <string>
#include <filesystem>
#include <glad/glad.h>
#include "Utilities/Logger.h"

class TextureBase {
public:
    TextureBase() = default;
    explicit TextureBase(std::filesystem::path filePath);
    virtual ~TextureBase();

    TextureBase(const TextureBase& other) = delete;
    TextureBase& operator=(const TextureBase& other) = delete;
    TextureBase(TextureBase&& other) noexcept;
    TextureBase& operator=(TextureBase&& other) noexcept;

    virtual void Bind(GLuint slot = 0) const = 0;
    virtual void Unbind(GLuint slot = 0) const = 0;

    int GetWidth() const noexcept { return m_Width; }
    int GetHeight() const noexcept { return m_Height; }
    GLuint GetRendererID() const noexcept { return m_RendererID; }
    const std::filesystem::path& GetFilePath() const noexcept { return m_FilePath; }

protected:
    void Release() noexcept;

    GLuint m_RendererID = 0;
    std::filesystem::path m_FilePath;
    int m_Width = 0;
    int m_Height = 0;
    int m_BPP = 0; 
};