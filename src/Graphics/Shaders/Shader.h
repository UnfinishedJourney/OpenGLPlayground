#pragma once

#include "BaseShader.h"
#include <unordered_map>
#include <filesystem>

class Shader : public BaseShader {
public:
    Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
        const std::filesystem::path& binaryPath = "");

    void ReloadShader() override;
    void BindUniformBlock(const std::string& blockName, GLuint bindingPoint);
    void BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint);

protected:
    void LoadShaders(bool reload = false);

private:
    std::unordered_map<GLenum, std::filesystem::path> m_ShaderStages;
};