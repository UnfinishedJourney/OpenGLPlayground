#pragma once

#include "BaseShader.h"

class ComputeShader : public BaseShader {
public:
    ComputeShader(std::filesystem::path sourcePath, std::filesystem::path binaryPath);

    void ReloadShader() override;

    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const;

protected:
    void LoadShader(bool bReload = false);
};
