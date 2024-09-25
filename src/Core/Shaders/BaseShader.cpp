#include "BaseShader.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/*BaseShader::BaseShader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}*/

BaseShader::BaseShader(const std::string& filepath)
    : m_FilePath(filepath), m_RendererID(0)
{
}

BaseShader::~BaseShader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

// Function to read shaders from a file
/*ShaderProgramSource BaseShader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1, COMPUTE = 2
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
            else if (line.find("compute") != std::string::npos)
            {
                type = ShaderType::COMPUTE;
            }
        }

        else if (line.find("#include") != std::string::npos)
        {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            std::string includeFile = line.substr(start, end - start);

            std::string includedCode = ParseIncludedShader(includeFile);

            ss[(int)type] << includedCode << '\n';
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str(), ss[2].str() };
}*/

std::string BaseShader::ParseIncludedShader(const std::string& includeFile)
{
    std::ifstream stream(includeFile);
    std::stringstream ss;
    std::string line;

    while (getline(stream, line))
    {
        if (line.find("#include") != std::string::npos)
        {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            std::string nestedIncludeFile = line.substr(start, end - start);

            std::string nestedIncludedCode = ParseIncludedShader(nestedIncludeFile);
            ss << nestedIncludedCode << '\n';
        }
        else
        {
            ss << line << '\n';
        }
    }

    return ss.str();
}

/*unsigned int BaseShader::CompileShader(unsigned int type, const std::string& source)
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
}*/

//unsigned int BaseShader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
//{
//    unsigned int program = glCreateProgram();
//    if (program == 0)
//    {
//        std::cerr << "Error creating program object." << std::endl;
//        exit(EXIT_FAILURE);
//    }
//
//    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
//    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
//
//    glAttachShader(program, vs);
//    glAttachShader(program, fs);
//    glLinkProgram(program);
//
//    /*GLint status;
//    glGetProgramiv(program, GL_LINK_STATUS, &status);
//    if (GL_FALSE == status) {
//        std::cerr << "Failed to link shader program!" << std::endl;
//        GLint logLen;
//        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
//        if (logLen > 0) {
//            std::string(logLen, ' ');
//            GLsizei written;
//            glGetProgramInfoLog(program, logLen, &written, &log[0]);
//            std::cerr << "Program log: " << std::endl << log;
//        }
//    }*/
//
//
//    glValidateProgram(program);
//    glDetachShader(program, vs);
//    glDetachShader(program, fs);
//
//    glDeleteShader(vs);
//    glDeleteShader(fs);
//
//    return program;
//}

void BaseShader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}
void BaseShader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void BaseShader::SetUniform(const std::string& name, float value) const 
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void BaseShader::SetUniform(const std::string& name, int value) const
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void BaseShader::SetUniform(const std::string& name, unsigned int value) const
{
    GLCall(glUniform1ui(GetUniformLocation(name), value));
}

void BaseShader::SetUniform(const std::string& name, const glm::vec3& value) const {
    GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
}

void BaseShader::SetUniform(const std::string& name, float v0, float v1, float v2) const
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void BaseShader::SetUniform(const std::string& name, const glm::vec4& value) const {
    GLCall(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
}

void BaseShader::SetUniform(const std::string& name, float v0, float v1, float v2, float v3) const
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void BaseShader::SetUniform(const std::string& name, const glm::mat3& value) const {
    GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]));
}

void BaseShader::SetUniform(const std::string& name, const glm::mat4& value) const {
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]));
}

int BaseShader::GetUniformLocation(const std::string& name) const
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache.at(name);

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}