#pragma once

#include "BaseShader.h"
#include <unordered_map>
#include <filesystem>

namespace graphics {

    /**
     * @brief Standard shader program supporting vertex, fragment, geometry, and tessellation.
     */
    class Shader : public BaseShader {
    public:
        Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
            const std::filesystem::path& binaryPath = "");
        void ReloadShader() override;

    private:
        void LoadShaders(bool reload = false);
        std::unordered_map<GLenum, std::filesystem::path> shaderStages_;
    };

} // namespace graphics