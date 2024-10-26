#pragma once

#include "BaseShader.h"

class Shader : public BaseShader {
public:
    Shader(std::filesystem::path sourcePath, std::filesystem::path binaryPath);

    void ReloadShader() override;

protected:
    void LoadShader();
};