#pragma once

#include "BaseShader.h"

class ComputeShader : public BaseShader {
public:
    explicit ComputeShader(const std::filesystem::path& filepath)
        : BaseShader(filepath)
    {
        LoadShader(filepath);
    }

    void Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) const;

protected:
    void LoadShader(const std::filesystem::path& filepath) override;
};