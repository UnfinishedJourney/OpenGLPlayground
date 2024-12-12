#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include "TextureData.h"
#include <glad/glad.h>
#include <string>
#include <vector>

class OpenGLTextureArray : public ITexture {
public:
    // Updated constructor comment:
    // If you have multiple files, pass them in filePaths.
    // If you have only one file with a sprite sheet, also pass it as a single element in filePaths.
    // Then use `frameCount` and `gridX`/`gridY` to slice it into multiple layers.
    OpenGLTextureArray(const std::vector<std::string>& filePaths, const TextureConfig& config,
        uint32_t totalFrames = 64, uint32_t gridX = 8, uint32_t gridY = 8);
    ~OpenGLTextureArray();
    void Bind(uint32_t unit) const override;
    void Unbind(uint32_t unit) const override;
    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
    bool IsBindless() const override { return m_IsBindless; }

private:
    GLuint m_TextureID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint64_t m_BindlessHandle = 0;
    bool m_IsBindless = false;
};