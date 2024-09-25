#include "ComputeShader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Constructor
ComputeShader::ComputeShader(const std::string& computeFilePath)
    : BaseShader(computeFilePath)
{
    ShaderProgramSource source = ParseShader(computeFilePath);
    m_RendererID = CreateShader(source);
}


// ParseShader implementation: parses a file with compute shader
ShaderProgramSource ComputeShader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, COMPUTE = 2
    };

    std::stringstream ss[3];
    ShaderType type = ShaderType::NONE;
    std::string line;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("compute") != std::string::npos)
            {
                type = ShaderType::COMPUTE;
            }
            else
            {
                type = ShaderType::NONE;
            }
        }
        else if (line.find("#include") != std::string::npos)
        {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            if (start == std::string::npos || end == std::string::npos)
            {
                std::cerr << "Invalid #include directive in file: " << filepath << std::endl;
                continue;
            }
            std::string includeFile = line.substr(start, end - start);

            std::string includedCode = ParseIncludedShader(includeFile);

            if (type != ShaderType::NONE)
                ss[(int)type] << includedCode << '\n';
        }
        else
        {
            if (type != ShaderType::NONE)
                ss[(int)type] << line << '\n';
        }
    }

    return { "", "", ss[2].str() }; // Only ComputeSource populated
}

// CompileShader implementation: compiles compute shader
unsigned int ComputeShader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // Check compilation status
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile compute shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// CreateShader implementation: links compute shader into a program
unsigned int ComputeShader::CreateShader(const ShaderProgramSource& source)
{
    unsigned int program = glCreateProgram();
    if (program == 0)
    {
        std::cerr << "Error creating compute shader program." << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int cs = CompileShader(GL_COMPUTE_SHADER, source.ComputeSource);
    if (cs == 0)
    {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, cs);
    glLinkProgram(program);

    // Check linking status
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, ' ');
        glGetProgramInfoLog(program, logLen, &logLen, &log[0]);
        std::cerr << "Failed to link compute shader program!" << std::endl;
        std::cerr << log << std::endl;
        glDeleteProgram(program);
        glDeleteShader(cs);
        return 0;
    }

    glValidateProgram(program);
    glDetachShader(program, cs);

    glDeleteShader(cs);

    return program;
}

// Dispatch the compute shader
void ComputeShader::Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) const
{
    Bind();
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    // Ensure all writes to buffers and images have been completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    Unbind();
}