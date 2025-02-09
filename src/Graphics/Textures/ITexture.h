#pragma once
#include <cstdint>

namespace Graphics {

    /**
     * @brief Abstract interface for a texture.
     */
    class ITexture {
    public:
        virtual ~ITexture() = default;

        /// Bind the texture to a texture unit.
        virtual void Bind(uint32_t unit) const = 0;
        /// Unbind the texture from a texture unit.
        virtual void Unbind(uint32_t unit) const = 0;

        virtual uint32_t GetWidth()  const = 0;
        virtual uint32_t GetHeight() const = 0;

        /// If using bindless textures, return the GPU handle.
        virtual uint64_t GetBindlessHandle() const = 0;
        /// Returns true if the texture is using bindless mode.
        virtual bool IsBindless() const = 0;
    };

} // namespace Graphics