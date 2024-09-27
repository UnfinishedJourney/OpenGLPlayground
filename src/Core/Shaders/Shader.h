#pragma once

#include "BaseShader.h"

class Shader : public BaseShader {
public:
    explicit Shader(const std::filesystem::path& filepath)
        : BaseShader(filepath)
    {
        LoadShader(filepath);
    }

protected:
    void LoadShader(const std::filesystem::path& filepath) override;
};