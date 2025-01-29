#pragma once

#include "BaseShader.h"
#include <filesystem>

/**
 * @brief Specialized class for compute shaders (GL_COMPUTE_SHADER).
 */
class ComputeShader : public BaseShader {
public:
    /**
     * @param shaderPath  The .comp file containing the compute shader source.
     * @param binaryPath  (Optional) path for saving/loading a program binary.
     */
    ComputeShader(const std::filesystem::path& shaderPath,
        const std::filesystem::path& binaryPath = "");

    /**
     * @brief Reload from file or binary.
     */
    void ReloadShader() override;

    /**
     * @brief Dispatch compute with given group sizes.
     */
    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

private:
    void LoadShader(bool reload = false);

    std::filesystem::path m_ShaderPath;
};