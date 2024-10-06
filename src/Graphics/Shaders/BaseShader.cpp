#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Utility.h"
#include "Utilities/Logger.h" 
#include "Resources/ShaderManager.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>

BaseShader::BaseShader(const std::filesystem::path& filepath)
    : m_Filepath(filepath)
{
}

BaseShader::~BaseShader() {
    auto logger = Logger::GetLogger();
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        logger->info("Deleted shader program with ID: {}", m_RendererID);
    }
}

void BaseShader::Bind() const {
    glUseProgram(m_RendererID);
    auto logger = Logger::GetLogger();
    logger->info("Shader program bound with ID: {}", m_RendererID);
}

void BaseShader::Unbind() const {
    glUseProgram(0);
    auto logger = Logger::GetLogger();
    logger->info("Shader program unbound.");
}

int BaseShader::GetUniformLocation(const std::string& name) const {
    auto logger = Logger::GetLogger();
    if (auto it = m_UniformLocationCache.find(name); it != m_UniformLocationCache.end()) {
        logger->debug("Cached location for uniform '{}': {}", name, it->second);
        return it->second;
    }
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) {
        logger->warn("Uniform '{}' does not exist or is not used.", name);
    }
    else {
        logger->debug("Found location for uniform '{}': {}", name, location);
    }
    m_UniformLocationCache[name] = location;
    return location;
}

unsigned int BaseShader::CompileShader(unsigned int type, const std::string& source) const {
    auto logger = Logger::GetLogger();
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
        logger->error("Shader compilation failed ({}):\n{}", shaderType, infoLog);
        throw std::runtime_error("Shader compilation failed (" + shaderType + "):\n" + infoLog);
    }
    logger->info("{} shader compiled successfully.",
        (type == GL_VERTEX_SHADER) ? "Vertex" :
        (type == GL_FRAGMENT_SHADER) ? "Fragment" : "Compute");
    return shader;
}

unsigned int BaseShader::LinkProgram(const std::vector<unsigned int>& shaders) const {
    auto logger = Logger::GetLogger();
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
        logger->error("Program linking failed:\n{}", infoLog);
        throw std::runtime_error("Program linking failed:\n" + infoLog);
    }

    for (auto shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
    logger->info("Shader program linked successfully with ID: {}", program);
    return program;
}

std::string BaseShader::ReadFile(const std::filesystem::path& filepath) const {
    auto logger = Logger::GetLogger();
    logger->info("Current working directory: {}", std::filesystem::current_path().generic_string());

    logger->info("Attempting to open shader file: {}", filepath.generic_string());

    if (!std::filesystem::exists(filepath)) {
        logger->error("Shader file does not exist: {}", filepath.string());
        throw std::runtime_error("Shader file does not exist: " + filepath.string());
    }

    std::ifstream file(filepath.generic_string(), std::ios::in);
    if (!file) {
        logger->error("Failed to open shader file: {}", filepath.string());
        throw std::runtime_error("Failed to open shader file: " + filepath.string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    logger->info("Shader file '{}' read successfully.", filepath.generic_string());
    return contents.str();
}

std::string BaseShader::ResolveIncludes(const std::string& source, const std::filesystem::path& directory) const {
    auto logger = Logger::GetLogger();
    std::istringstream stream(source);
    std::ostringstream processedSource;
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("#include") != std::string::npos) {
            size_t start = line.find("\"");
            size_t end = line.find("\"", start + 1);

            if (start == std::string::npos || end == std::string::npos) {
                logger->error("Wrong #include syntax: {}", line);
                throw std::runtime_error("Wrong #include: " + line);
            }

            std::string includePathStr = line.substr(start + 1, end - start - 1);
            std::filesystem::path includePath = directory / includePathStr;

            logger->info("Including file: {}", includePath.generic_string());

            std::string includedSource = ReadFile(includePath);

            includedSource = ResolveIncludes(includedSource, includePath.parent_path());

            processedSource << includedSource << '\n';
        }
        else {
            processedSource << line << '\n';
        }
    }

    return processedSource.str();
}

bool BaseShader::LoadBinary(const std::filesystem::path& binaryPath) {
    auto logger = Logger::GetLogger();
    logger->info("Attempting to load shader binary: {}", binaryPath.generic_string());

    // Open the binary file
    std::ifstream inStream(binaryPath, std::ios::binary);
    if (!inStream.is_open()) {
        logger->error("Failed to open shader binary file: {}", binaryPath.generic_string());
        return false;
    }

    // Read the binary format
    GLenum binaryFormat;
    inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));

    // Read the binary data
    std::vector<GLubyte> binaryData((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());
    inStream.close();

    if (binaryData.empty()) {
        logger->error("Shader binary file is empty: {}", binaryPath.generic_string());
        return false;
    }

    // Create the program object
    m_RendererID = glCreateProgram();
    if (m_RendererID == 0) {
        logger->error("Error creating shader program object.");
        return false;
    }

    // Load the binary data into the program
    glProgramBinary(m_RendererID, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));

    // Check for successful linking
    GLint status;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &status);
    if (GL_FALSE == status) {
        logger->error("Failed to load binary shader program!");
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
        return false;
    }

    logger->info("Successfully loaded shader binary.");
    return true;
}

void BaseShader::SaveBinary(const std::filesystem::path& binaryPath) const {
    auto logger = Logger::GetLogger();
    GLint formats;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
    logger->info("Number of binary formats supported by this driver: {}", formats);

    if (formats > 0) {
        GLint binaryLength;
        glGetProgramiv(m_RendererID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        logger->info("Program binary length: {} bytes", binaryLength);

        std::vector<GLubyte> binary(binaryLength);
        GLenum binaryFormat;
        glGetProgramBinary(m_RendererID, binaryLength, nullptr, &binaryFormat, binary.data());

        // Write the binary format first, so it can be read during loading
        std::ofstream outStream(binaryPath, std::ios::binary);
        if (!outStream.is_open()) {
            logger->error("Failed to open file for writing shader binary: {}", binaryPath.generic_string());
            return;
        }

        outStream.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
        outStream.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        outStream.close();

        logger->info("Shader binary saved to: {}", binaryPath.generic_string());
    }
    else {
        logger->error("No binary formats supported by this driver. Unable to save shader binary.");
    }
}

void BaseShader::SetUniform(const std::string& name, float value) const
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to float value: {}", name, value);
}

void BaseShader::SetUniform(const std::string& name, int value) const
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to int value: {}", name, value);
}

void BaseShader::SetUniform(const std::string& name, unsigned int value) const
{
    GLCall(glUniform1ui(GetUniformLocation(name), value));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to unsigned int value: {}", name, value);
}

void BaseShader::SetUniform(const std::string& name, const glm::vec2& value) const
{
    GLCall(glUniform2f(GetUniformLocation(name), value.x, value.y));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to vec2 value: ({}, {})", name, value.x, value.y);
}

void BaseShader::SetUniform(const std::string& name, const glm::vec3& value) const {
    GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to vec3 value: ({}, {}, {})", name, value.x, value.y, value.z);
}

void BaseShader::SetUniform(const std::string& name, float v0, float v1, float v2) const
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to vec3 values: ({}, {}, {})", name, v0, v1, v2);
}

void BaseShader::SetUniform(const std::string& name, const glm::vec4& value) const {
    GLCall(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to vec4 value: ({}, {}, {}, {})", name, value.x, value.y, value.z, value.w);
}

void BaseShader::SetUniform(const std::string& name, float v0, float v1, float v2, float v3) const
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to vec4 values: ({}, {}, {}, {})", name, v0, v1, v2, v3);
}

void BaseShader::SetUniform(const std::string& name, const glm::mat3& value) const {
    GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to mat3 value.", name);
}

void BaseShader::SetUniform(const std::string& name, const glm::mat4& value) const {
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]));
    auto logger = Logger::GetLogger();
    logger->debug("Set uniform '{}' to mat4 value.", name);
}