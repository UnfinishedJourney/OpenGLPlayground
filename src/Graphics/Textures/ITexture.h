#pragma once
#include <cstdint>

class ITexture {
public:
    virtual ~ITexture() = default;

    // Bind/unbind the texture at a given texture unit
    virtual void Bind(uint32_t unit) const = 0;
    virtual void Unbind(uint32_t unit) const = 0;

    // Basic info
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    // Bindless handle (if using ARB_bindless_texture)
    virtual uint64_t GetBindlessHandle() const = 0;
    virtual bool IsBindless() const = 0;
};