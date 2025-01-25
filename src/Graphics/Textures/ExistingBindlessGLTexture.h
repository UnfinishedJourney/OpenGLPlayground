#pragma once
#include "ITexture.h"
#include "TextureData.h"
#include "TextureConfig.h"
#include <glad/glad.h>
#include <cstdint>

class ExistingBindlessGLTexture : public ITexture {
public:
    ExistingBindlessGLTexture(GLuint id, int w, int h, bool makeBindless)
        : m_TextureID(id), m_Width(w), m_Height(h)
    {
        if (makeBindless && GLAD_GL_ARB_bindless_texture) {
            m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
            if (m_BindlessHandle) {
                glMakeTextureHandleResidentARB(m_BindlessHandle);
                m_IsBindless = true;
            }
        }
    }

    ~ExistingBindlessGLTexture() {
        // If bindless, make it non-resident
        if (m_IsBindless && m_BindlessHandle) {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
        }
        // Delete the actual texture
        if (m_TextureID) {
            glDeleteTextures(1, &m_TextureID);
        }
    }

    void Bind(uint32_t unit) const override {
        if (!m_IsBindless) {
            glBindTextureUnit(unit, m_TextureID);
        }
        // If bindless, the shader will use glUniformHandleui64ARB or layout(binding)
        // so we typically do nothing here
    }

    void Unbind(uint32_t unit) const override {
        if (!m_IsBindless) {
            glBindTextureUnit(unit, 0);
        }
    }

    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }

    uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
    bool IsBindless() const override { return m_IsBindless; }

private:
    GLuint   m_TextureID = 0;
    int      m_Width = 0;
    int      m_Height = 0;
    bool     m_IsBindless = false;
    uint64_t m_BindlessHandle = 0;
};
