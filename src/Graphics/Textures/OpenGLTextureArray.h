#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include "TextureData.h"
#include <vector>
#include <string>
#include <cstdint>
#include <glad/glad.h>

namespace graphics {

    /**
     * @brief Represents a 2D texture array (e.g. a sprite sheet sliced into frames).
     *
     * The constructor slices a single image (or could later combine multiple files)
     * into layers of a GL texture array.
     */
    class OpenGLTextureArray : public ITexture {
    public:
        OpenGLTextureArray(const std::vector<std::string>& filePaths,
            const TextureConfig& config,
            uint32_t totalFrames = 64,
            uint32_t gridX = 8,
            uint32_t gridY = 8);
        ~OpenGLTextureArray() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth()  const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
        bool IsBindless()        const override { return m_IsBindless; }

    private:
        GLuint m_TextureID = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint64_t m_BindlessHandle = 0;
        bool m_IsBindless = false;
    };

} // namespace graphics
