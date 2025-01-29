#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include <glad/glad.h>

#include "ITexture.h"
#include "TextureData.h"
#include "TextureConfig.h"
#include "Graphics/Buffers/BufferDeleter.h" // or a dedicated TextureDeleter if you prefer a separate file

/**
 * @brief Represents a 2D texture array in OpenGL.
 *
 * The constructor can slice one large sprite sheet into multiple frames,
 * or if you have multiple files (not currently supported in this example),
 * it would combine them into layers of the array.
 */
class OpenGLTextureArray : public ITexture {
public:
    /**
     * @param filePaths     If you have multiple files, pass them in.
     *                      (Currently only the first is used.)
     * @param config        Texture creation parameters.
     * @param totalFrames   How many frames/layers in total.
     * @param gridX, gridY  If you're slicing a single sprite sheet, use these
     *                      to define sub-images across the sheet.
     */
    OpenGLTextureArray(const std::vector<std::string>& filePaths,
        const TextureConfig& config,
        uint32_t totalFrames = 64,
        uint32_t gridX = 8,
        uint32_t gridY = 8);

    ~OpenGLTextureArray() override;

    // ITexture interface
    void Bind(uint32_t unit)   const override;
    void Unbind(uint32_t unit) const override;

    uint32_t GetWidth()  const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }

    uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
    bool     IsBindless()        const override { return m_IsBindless; }

private:
    // RAII for texture handle
    std::unique_ptr<GLuint, TextureDeleter> m_TextureIDPtr;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    uint64_t m_BindlessHandle = 0;
    bool     m_IsBindless = false;
};