#pragma once

#include "BaseShader.h"
#include <unordered_map>
#include <filesystem>

namespace Graphics {

    /**
     * @brief A standard shader program class supporting vertex, fragment, geometry, tessellation
     *
     * Constructed by supplying a map of shader stage enums to source file paths.
     */
    class Shader : public BaseShader {
    public:
        /**
         * @brief Constructor.
         * @param ShaderStages Map from GLenum (e.g. GL_VERTEX_SHADER) to shader source file path.
         * @param BinaryPath binary path for caching.
         */
        Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
            const std::filesystem::path& binaryPath = "");

        /**
         * @brief Reloads the shader (from source or binary).
         */
        void ReloadShader() override;

    private:
        void LoadShaders(bool reload = false);
        std::unordered_map<GLenum, std::filesystem::path> m_ShaderStages;
    };

} // namespace Graphics