#pragma once

#include "BaseShader.h"
#include <filesystem>

class ComputeShader : public BaseShader {
public:
    ComputeShader(const std::filesystem::path& shaderPath, const std::filesystem::path& binaryPath = "");
    void ReloadShader() override;
    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

protected:
    void LoadShader(bool reload = false);

private:
    std::filesystem::path m_ShaderPath;
};