#pragma once

#include "BaseShader.h"
#include <filesystem>

namespace graphics {

    /**
     * @brief Specialized class for compute shaders.
     */
    class ComputeShader : public BaseShader {
    public:
        ComputeShader(const std::filesystem::path& shaderPath, const std::filesystem::path& binaryPath = "");
        void ReloadShader() override;
        void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

    private:
        void LoadShader(bool reload = false);
        std::filesystem::path shaderPath_;  ///< Path to compute shader source.
    };

} // namespace graphics