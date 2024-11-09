#include "BaseShader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

BaseShader::BaseShader(std::filesystem::path sourcePath, std::filesystem::path binaryPath)
    : m_SourcePath(std::move(sourcePath)), m_BinaryPath(std::move(binaryPath)) {
    // Constructor doesn't load shaders immediately
}

BaseShader::~BaseShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        Logger::GetLogger()->info("Deleted shader program with ID: {}", m_RendererID);
    }
}

void BaseShader::Bind() const {
    glUseProgram(m_RendererID);
}

void BaseShader::Unbind() const {
    glUseProgram(0);
}

GLint BaseShader::GetUniformLocation(std::string_view name) const {
    std::string nameStr(name);
    if (auto it = m_UniformLocationCache.find(nameStr); it != m_UniformLocationCache.end()) {
        return it->second;
    }
    GLint location = glGetUniformLocation(m_RendererID, nameStr.c_str());
    if (location == -1) {
        Logger::GetLogger()->warn("Uniform '{}' not found in shader program ID {}.", nameStr, m_RendererID);
    }
    m_UniformLocationCache[nameStr] = location;
    return location;
}

GLuint BaseShader::CompileShader(GLenum shaderType, const std::string& source) const {
    std::string shaderTypeStr = (shaderType == GL_VERTEX_SHADER) ? "vertex" :
        (shaderType == GL_FRAGMENT_SHADER) ? "fragment" :
        (shaderType == GL_COMPUTE_SHADER) ? "compute" : "unknown";

    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::string infoLog(static_cast<size_t>(maxLength), '\0');
        glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

        glDeleteShader(shader);
        Logger::GetLogger()->error("{} shader compilation failed:\n{}", shaderTypeStr, infoLog);
        throw std::runtime_error(shaderTypeStr + " shader compilation failed:\n" + infoLog);
    }

    Logger::GetLogger()->info("{} shader compiled successfully.", shaderTypeStr);

    return shader;
}

GLuint BaseShader::LinkProgram(const std::vector<GLuint>& shaders) const {
    GLuint program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::string infoLog(static_cast<size_t>(maxLength), '\0');
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(program);
        for (auto shader : shaders) {
            glDeleteShader(shader);
        }
        Logger::GetLogger()->error("Program linking failed:\n{}", infoLog);
        throw std::runtime_error("Program linking failed:\n" + infoLog);
    }

    for (auto shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }

    Logger::GetLogger()->info("Shader program linked successfully with ID: {}", program);
    return program;
}

std::string BaseShader::ReadFile(const std::filesystem::path& filepath) const {
    if (!std::filesystem::exists(filepath)) {
        Logger::GetLogger()->error("Shader file does not exist: {}", filepath.string());
        throw std::runtime_error("Shader file does not exist: " + filepath.string());
    }

    std::ifstream file(filepath, std::ios::in);
    if (!file) {
        Logger::GetLogger()->error("Failed to open shader file: {}", filepath.string());
        throw std::runtime_error("Failed to open shader file: " + filepath.string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    Logger::GetLogger()->info("Shader file '{}' read successfully.", filepath.string());
    return contents.str();
}

std::string BaseShader::ResolveIncludes(const std::string& source, const std::filesystem::path& directory, std::unordered_set<std::string>& includedFiles) const {
    std::istringstream stream(source);
    std::ostringstream processedSource;
    std::string line;
    bool inBlockComment = false;

    while (std::getline(stream, line)) {
        std::string originalLine = line;
        std::string trimmedLine = line;
        // Remove leading whitespace
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));

        // Handle block comments
        if (inBlockComment) {
            size_t endComment = trimmedLine.find("*/");
            if (endComment != std::string::npos) {
                inBlockComment = false;
                trimmedLine = trimmedLine.substr(endComment + 2);
            }
            else {
                continue; // Skip line inside block comment
            }
        }

        // Check for start of block comment
        size_t startBlockComment = trimmedLine.find("/*");
        if (startBlockComment != std::string::npos) {
            inBlockComment = true;
            trimmedLine = trimmedLine.substr(0, startBlockComment);
        }

        // Remove single-line comments
        size_t singleLineComment = trimmedLine.find("//");
        if (singleLineComment != std::string::npos) {
            trimmedLine = trimmedLine.substr(0, singleLineComment);
        }

        // Trim again after removing comments
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
        trimmedLine.erase(trimmedLine.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines
        if (trimmedLine.empty()) {
            processedSource << originalLine << '\n'; // Preserve original line formatting
            continue;
        }

        // Process includes
        if (trimmedLine.find("#include") == 0) {
            size_t start = trimmedLine.find_first_of("\"<");
            size_t end = trimmedLine.find_first_of("\">", start + 1);
            if (start == std::string::npos || end == std::string::npos) {
                Logger::GetLogger()->error("Invalid #include syntax: {}", originalLine);
                throw std::runtime_error("Invalid #include syntax: " + originalLine);
            }

            std::string includePathStr = trimmedLine.substr(start + 1, end - start - 1);
            std::filesystem::path includePath = directory / includePathStr;

            // Normalize the path
            includePath = std::filesystem::weakly_canonical(includePath);

            // Check if the file has already been included
            if (includedFiles.find(includePath.string()) != includedFiles.end()) {
                continue; // Skip including again
            }
            includedFiles.insert(includePath.string());

            Logger::GetLogger()->debug("Including file: {}", includePath.string());

            std::string includedSource = ReadFile(includePath);
            includedSource = ResolveIncludes(includedSource, includePath.parent_path(), includedFiles);
            processedSource << includedSource << '\n';
        }
        else {
            processedSource << originalLine << '\n';
        }
    }

    return processedSource.str();
}

bool BaseShader::LoadBinary() {
    if (m_BinaryPath.empty()) {
        return false;
    }

    if (!std::filesystem::exists(m_BinaryPath)) {
        Logger::GetLogger()->warn("Shader binary file does not exist: {}", m_BinaryPath.string());
        return false;
    }

    std::ifstream inStream(m_BinaryPath, std::ios::binary);
    if (!inStream) {
        Logger::GetLogger()->error("Failed to open shader binary file: {}", m_BinaryPath.string());
        return false;
    }

    GLenum binaryFormat;
    inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));

    std::vector<GLubyte> binaryData((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());
    if (binaryData.empty()) {
        Logger::GetLogger()->error("Shader binary file is empty: {}", m_BinaryPath.string());
        return false;
    }

    m_RendererID = glCreateProgram();
    glProgramBinary(m_RendererID, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));

    GLint success = GL_FALSE;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);
        std::string infoLog(static_cast<size_t>(maxLength), '\0');
        glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
        Logger::GetLogger()->error("Failed to load shader binary from '{}':\n{}", m_BinaryPath.string(), infoLog);
        return false;
    }

    Logger::GetLogger()->info("Successfully loaded shader binary from '{}'.", m_BinaryPath.string());
    return true;
}

void BaseShader::SaveBinary() const {
    if (m_BinaryPath.empty()) {
        return;
    }

    GLint formats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
    if (formats == 0) {
        Logger::GetLogger()->error("Driver does not support program binary formats.");
        return;
    }

    GLint binaryLength = 0;
    glGetProgramiv(m_RendererID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    std::vector<GLubyte> binary(static_cast<size_t>(binaryLength));
    GLenum binaryFormat = 0;

    glGetProgramBinary(m_RendererID, binaryLength, nullptr, &binaryFormat, binary.data());

    std::ofstream outStream(m_BinaryPath, std::ios::binary);
    if (!outStream) {
        Logger::GetLogger()->error("Failed to open file for writing shader binary: {}", m_BinaryPath.string());
        return;
    }

    outStream.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
    outStream.write(reinterpret_cast<const char*>(binary.data()), binary.size());

    Logger::GetLogger()->info("Shader binary saved to '{}'.", m_BinaryPath.string());
}

void BaseShader::SetUniform(std::string_view name, float value) const {
    glProgramUniform1f(m_RendererID, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, int value) const {
    glProgramUniform1i(m_RendererID, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, unsigned int value) const {
    glProgramUniform1ui(m_RendererID, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, const glm::vec2& value) const {
    glProgramUniform2fv(m_RendererID, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::vec3& value) const {
    glProgramUniform3fv(m_RendererID, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::vec4& value) const {
    glProgramUniform4fv(m_RendererID, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::mat3& value) const {
    glProgramUniformMatrix3fv(m_RendererID, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::mat4& value) const {
    glProgramUniformMatrix4fv(m_RendererID, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}