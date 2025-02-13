#include "ExistingBindlessGLTexture.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>

namespace graphics {

    ExistingBindlessGLTexture::ExistingBindlessGLTexture(GLuint id, int w, int h, bool makeBindless)
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

    ExistingBindlessGLTexture::~ExistingBindlessGLTexture() {
        if (m_IsBindless && m_BindlessHandle) {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
        }
        if (m_TextureID) {
            GLCall(glDeleteTextures(1, &m_TextureID));
        }
    }

    void ExistingBindlessGLTexture::Bind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, m_TextureID));
        }
    }

    void ExistingBindlessGLTexture::Unbind(uint32_t unit) const {
        if (!m_IsBindless) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace graphics