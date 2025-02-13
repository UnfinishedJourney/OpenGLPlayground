#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include <vector>
#include <string>
#include <glad/glad.h>
#include <cstdint>

namespace graphics {

    /**
     * @brief Represents a 2D texture array.
     *
     * Loads a sprite sheet and slices it into layers stored in a GL texture array.
     */
    class OpenGLTextureArray : public ITexture {
    public:
        /**
         * @brief Constructs a texture array by slicing a single sprite sheet.
         * @param file_paths A vector of file paths (only one file is supported currently).
         * @param config Texture configuration.
         * @param total_frames Total number of frames.
         * @param grid_x Number of columns in the sprite sheet.
         * @param grid_y Number of rows in the sprite sheet.
         * @throws std::runtime_error if texture creation fails.
         */
        OpenGLTextureArray(const std::vector<std::string>& file_paths,
            const TextureConfig& config,
            uint32_t total_frames = 64,
            uint32_t grid_x = 8,
            uint32_t grid_y = 8);
        ~OpenGLTextureArray() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth() const override { return width_; }
        uint32_t GetHeight() const override { return height_; }
        uint64_t GetBindlessHandle() const override { return bindless_handle_; }
        bool IsBindless() const override { return is_bindless_; }

    private:
        GLuint texture_id_ = 0;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        uint64_t bindless_handle_ = 0;
        bool is_bindless_ = false;
    };

} // namespace graphics