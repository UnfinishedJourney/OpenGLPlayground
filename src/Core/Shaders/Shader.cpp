#include "Shader.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Constructor
Shader::Shader(const std::string& filepath)
    : BaseShader(filepath)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source);
}

// ParseShader implementation: parses a file with vertex and fragment shaders
ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    std::string line;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
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

    return { ss[0].str(), ss[1].str(), "" }; // ComputeSource empty
}

// CompileShader implementation: compiles vertex or fragment shader
unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
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
        std::cout << "Failed to compile " <<
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
            " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// CreateShader implementation: links vertex and fragment shaders into a program
unsigned int Shader::CreateShader(const ShaderProgramSource& source)
{
    unsigned int program = glCreateProgram();
    if (program == 0)
    {
        std::cerr << "Error creating program object." << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, source.VertexSource);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, source.FragmentSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
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
        std::cerr << "Failed to link shader program!" << std::endl;
        std::cerr << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glValidateProgram(program);
    glDetachShader(program, vs);
    glDetachShader(program, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}