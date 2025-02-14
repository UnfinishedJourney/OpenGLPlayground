#pragma once
#include <cstdint>

namespace graphics {

    class ITexture {
    public:
        virtual ~ITexture() = default;
        virtual void Bind(uint32_t unit) const = 0;
        virtual void Unbind(uint32_t unit) const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint64_t GetBindlessHandle() const = 0;
        virtual bool IsBindless() const = 0;
    };

} // namespace graphics