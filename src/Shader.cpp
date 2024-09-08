#include "Shader.h"
#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Shader::Shader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

// Function to read shaders from a file
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
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

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

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    if (program == 0)
    {
        std::cerr << "Error creating program object." << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    /*GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (GL_FALSE == status) {
        std::cerr << "Failed to link shader program!" << std::endl;
        GLint logLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0) {
            std::string(logLen, ' ');
            GLsizei written;
            glGetProgramInfoLog(program, logLen, &written, &log[0]);
            std::cerr << "Program log: " << std::endl << log;
        }
    }*/


    glValidateProgram(program);
    glDetachShader(program, vs);
    glDetachShader(program, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}
void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniformf(const std::string& name, float v) const
{
    GLCall(glUniform1f(GetUniformLocation(name), v));
}

//Set uniforms
void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

int Shader::GetUniformLocation(const std::string& name) const
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache.at(name);

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}