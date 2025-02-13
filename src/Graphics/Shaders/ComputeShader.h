#pragma once

#include "BaseShader.h"
#include <filesystem>

namespace graphics {

    /**
     * @brief Specialized class for compute shaders.
     *
     * Loads a compute shader from a .comp file and supports binary caching.
     */
    class ComputeShader : public BaseShader {
    public:
        /**
         * @brief Constructor.
         * @param ShaderPath Path to the compute shader source (.comp file).
         * @param BinaryPath Optional binary path.
         */
        ComputeShader(const std::filesystem::path& shaderPath,
            const std::filesystem::path& binaryPath = "");

        /**
         * @brief Reloads the compute shader from source or binary.
         */
        void ReloadShader() override;

        /**
         * @brief Dispatches the compute shader.
         * @param NumGroupsX Number of work groups in X.
         * @param NumGroupsY Number of work groups in Y.
         * @param NumGroupsZ Number of work groups in Z.
         */
        void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

    private:
        void LoadShader(bool reload = false);
        std::filesystem::path m_ShaderPath;
    };

} // namespace graphics