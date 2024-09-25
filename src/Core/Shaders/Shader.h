#pragma once

#include <string>
#include <unordered_map>

#include "BaseShader.h"
#include "glm.hpp"



class Shader : public BaseShader
{
public:
    // Constructor takes path to shader file containing vertex and fragment shaders
    Shader(const std::string& filepath);

    virtual ~Shader() override = default;

protected:
    // Implement ParseShader, CompileShader, CreateShader
    virtual ShaderProgramSource ParseShader(const std::string& filepath) override;
    virtual unsigned int CompileShader(unsigned int type, const std::string& source) override;
    unsigned int CreateShader(const ShaderProgramSource& source);
};