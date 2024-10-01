#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Utility.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

BaseShader::BaseShader(const std::filesystem::path& filepath)
    : m_Filepath(filepath)
{
}

BaseShader::~BaseShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
    }
}

void BaseShader::Bind() const {
    glUseProgram(m_RendererID);
}

void BaseShader::Unbind() const {
    glUseProgram(0);
}

int BaseShader::GetUniformLocation(const std::string& name) const {
    if (auto it = m_UniformLocationCache.find(name); it != m_UniformLocationCache.end()) {
        return it->second;
    }
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist or is not used.\n";
    }
    m_UniformLocationCache[name] = location;
    return location;
}

unsigned int BaseShader::CompileShader(unsigned int type, const std::string& source) const {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check compilation status
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string infoLog(length, ' ');
        glGetShaderInfoLog(shader, length, &length, &infoLog[0]);
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" :
            (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "COMPUTE";
        throw std::runtime_error("Shader compilation failed (" + shaderType + "):\n" + infoLog);
    }
    return shader;
}

unsigned int BaseShader::LinkProgram(const std::vector<unsigned int>& shaders) const {
    unsigned int program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string infoLog(length, ' ');
        glGetProgramInfoLog(program, length, &length, &infoLog[0]);
        throw std::runtime_error("Program linking failed:\n" + infoLog);
    }

    for (auto shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
    return program;
}

std::string BaseShader::ReadFile(const std::filesystem::path& filepath) const {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + filepath.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string BaseShader::ResolveIncludes(const std::string& source, const std::filesystem::path& directory) const {
    std::istringstream stream(source);
    std::ostringstream processedSource;
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("#include") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            std::string includePath = line.substr(start, end - start);
            auto fullPath = directory / includePath;
            std::string includedSource = ReadFile(fullPath);
            includedSource = ResolveIncludes(includedSource, fullPath.parent_path());
            processedSource << includedSource << '\n';
        }
        else {
            processedSource << line << '\n';
        }
    }
    return processedSource.str();
}

bool BaseShader::LoadBinary(const std::filesystem::path& binaryPath) {
    std::cout << "Attempting to load shader binary: " << binaryPath << std::endl;

    // Open the binary file
    std::ifstream inStream(binaryPath, std::ios::binary);
    if (!inStream.is_open()) {
        std::cerr << "Failed to open shader binary file: " << binaryPath << std::endl;
        return false;
    }

    // Read the binary format
    GLenum binaryFormat;
    inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));

    // Read the binary data
    std::vector<GLubyte> binaryData((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());
    inStream.close();

    if (binaryData.empty()) {
        std::cerr << "Shader binary file is empty: " << binaryPath << std::endl;
        return false;
    }

    // Create the program object
    m_RendererID = glCreateProgram();
    if (m_RendererID == 0) {
        std::cerr << "Error creating shader program object." << std::endl;
        return false;
    }

    // Load the binary data into the program
    glProgramBinary(m_RendererID, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));

    // Check for successful linking
    GLint status;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &status);
    if (GL_FALSE == status) {
        std::cerr << "Failed to load binary shader program!" << std::endl;
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
        return false;
    }

    std::cout << "Successfully loaded shader binary." << std::endl;
    return true;
}

void BaseShader::SaveBinary(const std::filesystem::path& binaryPath) const {
    GLint formats;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
    std::cout << "Number of binary formats supported by this driver: " << formats << std::endl;

    if (formats > 0) {
        GLint binaryLength;
        glGetProgramiv(m_RendererID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        std::cout << "Program binary length: " << binaryLength << " bytes" << std::endl;

        std::vector<GLubyte> binary(binaryLength);
        GLenum binaryFormat;
        glGetProgramBinary(m_RendererID, binaryLength, nullptr, &binaryFormat, binary.data());

        // Write the binary format first, so it can be read during loading
        std::ofstream outStream(binaryPath, std::ios::binary);
        if (!outStream.is_open()) {
            std::cerr << "Failed to open file for writing shader binary: " << binaryPath << std::endl;
            return;
        }

        outStream.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
        outStream.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        outStream.close();

        std::cout << "Shader binary saved to: " << binaryPath << std::endl;
    }
    else {
        std::cerr << "No binary formats supported by this driver. Unable to save shader binary." << std::endl;
    }
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
