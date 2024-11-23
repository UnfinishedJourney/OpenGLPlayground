#pragma once

#include <string>
#include <filesystem>
#include <glad/glad.h>
#include <memory>
#include "Utilities/Logger.h"
#include "Graphics/Buffers/BufferDeleter.h"

class TextureBase {
public:
    TextureBase()
        : m_RendererIDPtr(new GLuint(0), TextureDeleter())
    {};
    virtual ~TextureBase() = default;

    // Move semantics
    TextureBase(TextureBase&& other) noexcept = default;
    TextureBase& operator=(TextureBase&& other) noexcept = default;

    // Delete copy semantics
    TextureBase(const TextureBase& other) = delete;
    TextureBase& operator=(const TextureBase& other) = delete;

    virtual void Bind(GLuint slot = 0) const = 0;
    virtual void Unbind(GLuint slot = 0) const = 0;

    // Accessors
    int GetWidth() const noexcept { return m_Width; }
    int GetHeight() const noexcept { return m_Height; }
    GLuint GetRendererID() const noexcept { return m_RendererIDPtr ? *m_RendererIDPtr : 0; }
    const std::filesystem::path& GetFilePath() const noexcept { return m_FilePath; }

protected:
    std::unique_ptr<GLuint, TextureDeleter> m_RendererIDPtr;
    std::filesystem::path m_FilePath;
    int m_Width = 0;
    int m_Height = 0;
    int m_BPP = 0;

    void Release() noexcept {
        if (m_RendererIDPtr && *m_RendererIDPtr != 0) {
            m_RendererIDPtr.reset();
            Logger::GetLogger()->info("Released texture resource.");
        }
    }
};