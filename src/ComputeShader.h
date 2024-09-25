#pragma once

#include <string>
#include <unordered_map>

#include "BaseShader.h"
#include "glm.hpp"



class ComputeShader : public BaseShader
{
public:
    // Constructor takes path to compute shader file
    ComputeShader(const std::string& computeFilePath);

    virtual ~ComputeShader() override = default;

    // Dispatch the compute shader
    void Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) const;

protected:
    // Implement ParseShader, CompileShader, CreateShader
    virtual ShaderProgramSource ParseShader(const std::string& filepath) override;
    virtual unsigned int CompileShader(unsigned int type, const std::string& source) override;
    unsigned int CreateShader(const ShaderProgramSource& source);
};