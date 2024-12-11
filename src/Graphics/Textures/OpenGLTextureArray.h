#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include "TextureData.h"
#include <glad/glad.h>
#include <string>
#include <vector>

class OpenGLTextureArray : public ITexture {
public:
    OpenGLTextureArray(const std::vector<std::string>& filePaths, const TextureConfig& config);
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