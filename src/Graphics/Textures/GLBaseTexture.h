#pragma once
#include "ITexture.h"
#include <glad/glad.h>
#include <cstdint>

namespace graphics {

    class GLBaseTexture : public ITexture {
    public:
        GLBaseTexture() = default;
        virtual ~GLBaseTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth()  const override { return width_; }
        uint32_t GetHeight() const override { return height_; }
        uint64_t GetBindlessHandle() const override { return bindless_handle_; }
        bool     IsBindless()        const override { return is_bindless_; }

    protected:
        void MakeBindlessIfNeeded(bool useBindless);

    protected:
        GLuint   texture_id_ = 0;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        uint64_t bindless_handle_ = 0;
        bool     is_bindless_ = false;
    };

} // namespace graphics