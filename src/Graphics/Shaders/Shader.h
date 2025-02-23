#pragma once

#include "BaseShader.h"
#include <unordered_map>
#include <filesystem>

namespace graphics {

    /**
     * @brief A standard shader program class supporting vertex, fragment, geometry, and tessellation.
     *
     * Constructed by supplying a map of shader stage enums to source file paths.
     */
    class Shader : public BaseShader {
    public:
        /**
         * @brief Constructor.
         * @param shaderStages Map from GLenum (e.g. GL_VERTEX_SHADER) to shader source file path.
         * @param binaryPath Binary path for caching.
         */
        Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
            const std::filesystem::path& binaryPath = "");

        /**
         * @brief Reloads the shader (from source or binary).
         */
        void ReloadShader() override;

    private:
        void LoadShaders(bool reload = false);
        std::unordered_map<GLenum, std::filesystem::path> m_ShaderStages_;
    };

} // namespace graphics