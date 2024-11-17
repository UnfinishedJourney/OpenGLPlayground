#pragma once

#include "BaseShader.h"
#include <unordered_map>

class Shader : public BaseShader {
public:
    Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderPaths, std::filesystem::path binaryPath);

    void ReloadShader() override;

    // New methods for binding blocks
    void BindUniformBlock(const std::string& blockName, GLuint bindingPoint);
    void BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint);

protected:
    void LoadShader(bool bReload = false);

private:
    std::unordered_map<GLenum, std::filesystem::path> m_ShaderPaths;
};