#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include <array>
#include <filesystem>
#include <glad/glad.h>
#include <cstdint>

namespace graphics {

    /**
     * @brief Represents an OpenGL cube map texture.
     *
     * Supports loading from 6 individual face files or from multiple mip levels.
     */
    class OpenGLCubeMapTexture : public ITexture {
    public:
        /**
         * @brief Constructs a cube map texture from 6 face image files.
         * @param faces Array of 6 file paths.
         * @param config Texture configuration.
         * @throws std::runtime_error if any face fails to load.
         */
        OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
            const TextureConfig& config);

        /**
         * @brief Constructs a cube map texture from multiple mip levels.
         * @param mip_faces Vector of 6-face arrays, one per mip level.
         * @param config Texture configuration.
         * @throws std::runtime_error if any face fails to load.
         */
        OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces,
            const TextureConfig& config);

        ~OpenGLCubeMapTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth() const override { return width_; }
        uint32_t GetHeight() const override { return height_; }
        uint64_t GetBindlessHandle() const override { return bindless_handle_; }
        bool IsBindless() const override { return is_bindless_; }

    private:
        void InitializeCubeMap(const std::array<std::filesystem::path, 6>& faces, const TextureConfig& config);
        void InitializeCubeMapMip(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces, const TextureConfig& config);

        GLuint texture_id_ = 0;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        uint64_t bindless_handle_ = 0;
        bool is_bindless_ = false;
    };

} // namespace graphics