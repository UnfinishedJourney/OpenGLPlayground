#include "GLBaseTexture.h"
#include <glad/glad.h>

namespace graphics {

    GLBaseTexture::~GLBaseTexture() {
        if (is_bindless_ && bindless_handle_) {
            glMakeTextureHandleNonResidentARB(bindless_handle_);
        }
        if (texture_id_) {
            glDeleteTextures(1, &texture_id_);
        }
    }

    void GLBaseTexture::Bind(uint32_t unit) const {
        if (!is_bindless_)
            glBindTextureUnit(unit, texture_id_);
    }

    void GLBaseTexture::Unbind(uint32_t unit) const {
        if (!is_bindless_)
            glBindTextureUnit(unit, 0);
    }

    void GLBaseTexture::MakeBindlessIfNeeded(bool useBindless) {
        if (useBindless && GLAD_GL_ARB_bindless_texture) {
            bindless_handle_ = glGetTextureHandleARB(texture_id_);
            if (bindless_handle_) {
                glMakeTextureHandleResidentARB(bindless_handle_);
                is_bindless_ = true;
            }
        }
    }

} // namespace graphics