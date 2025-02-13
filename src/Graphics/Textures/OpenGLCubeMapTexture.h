#pragma once
#include "ITexture.h"
#include "TextureConfig.h"
#include <array>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <glad/glad.h>

namespace graphics {

    /**
     * @brief Represents an OpenGL cube map texture.
     *
     * Supports both single mip-level cube maps (6 files) and
     * cube maps with multiple mip levels.
     */
    class OpenGLCubeMapTexture : public ITexture {
    public:
        /**
         * @brief Constructs a cube map texture from 6 file paths (one per face).
         */
        OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
            const TextureConfig& config);

        /**
         * @brief Constructs a cube map texture from multiple mip levels.
         * Each mip level is an array of 6 file paths.
         */
        OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mipFaces,
            const TextureConfig& config);

        ~OpenGLCubeMapTexture() override;

        void Bind(uint32_t unit) const override;
        void Unbind(uint32_t unit) const override;

        uint32_t GetWidth()  const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return m_BindlessHandle; }
        bool     IsBindless()        const override { return m_IsBindless; }

    private:
        GLuint m_TextureID = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint64_t m_BindlessHandle = 0;
        bool m_IsBindless = false;
    };

} // namespace graphics