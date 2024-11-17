#pragma once

#include "BaseShader.h"

class ComputeShader : public BaseShader {
public:
    ComputeShader(const std::filesystem::path& shaderPath, std::filesystem::path binaryPath);

    void ReloadShader() override;

    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

protected:
    void LoadShader(bool bReload = false);

private:
    std::filesystem::path m_ShaderPath;
};