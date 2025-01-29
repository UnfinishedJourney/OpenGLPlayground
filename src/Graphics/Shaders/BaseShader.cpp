#include "BaseShader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

// GLM
#include <glm/gtc/type_ptr.hpp>

// Internal utilities/log
#include "Utilities/Logger.h"

BaseShader::BaseShader(const std::filesystem::path& binaryPath)
    : m_BinaryPath(binaryPath),
    m_RendererIDPtr(new GLuint(0), ShaderDeleter())
{
}

void BaseShader::Bind() const noexcept
{
    glUseProgram(*m_RendererIDPtr);
}

void BaseShader::Unbind() const noexcept
{
    glUseProgram(0);
}

GLint BaseShader::GetUniformLocation(std::string_view name) const
{
    std::string uniformName(name);

    // Check cache
    if (auto it = m_UniformLocationCache.find(uniformName);
        it != m_UniformLocationCache.end())
    {
        return it->second;
    }

    // Query from GL
    GLint location = glGetUniformLocation(*m_RendererIDPtr, uniformName.c_str());
    if (location == -1) {
        Logger::GetLogger()->warn("Uniform '{}' not found in shader program ID {}.",
            uniformName, *m_RendererIDPtr);
    }

    m_UniformLocationCache[uniformName] = location;
    return location;
}

GLuint BaseShader::CompileShader(GLenum shaderType, const std::string& source) const
{
    // Create and compile
    GLuint shader = glCreateShader(shaderType);
    const char* srcPtr = source.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    // Check result
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        std::string infoLog(static_cast<size_t>(infoLen), '\0');
        glGetShaderInfoLog(shader, infoLen, &infoLen, infoLog.data());
        glDeleteShader(shader);

        // Derive a human-readable shader type string
        std::string typeStr = "UNKNOWN";
        switch (shaderType) {
        case GL_VERTEX_SHADER:          typeStr = "VERTEX"; break;
        case GL_FRAGMENT_SHADER:        typeStr = "FRAGMENT"; break;
        case GL_GEOMETRY_SHADER:        typeStr = "GEOMETRY"; break;
        case GL_TESS_CONTROL_SHADER:    typeStr = "TESS_CONTROL"; break;
        case GL_TESS_EVALUATION_SHADER: typeStr = "TESS_EVALUATION"; break;
        case GL_COMPUTE_SHADER:         typeStr = "COMPUTE"; break;
        default: break;
        }

        Logger::GetLogger()->error("{} shader compilation failed:\n{}", typeStr, infoLog);
        throw std::runtime_error(typeStr + " shader compilation failed:\n" + infoLog);
    }

    return shader;
}

GLuint BaseShader::LinkProgram(const std::vector<GLuint>& shaders) const
{
    GLuint program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    // Check link status
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        std::string infoLog(static_cast<size_t>(infoLen), '\0');
        glGetProgramInfoLog(program, infoLen, &infoLen, infoLog.data());

        glDeleteProgram(program);
        for (auto shader : shaders) {
            glDeleteShader(shader);
        }

        Logger::GetLogger()->error("Shader program linking failed:\n{}", infoLog);
        throw std::runtime_error("Shader program linking failed:\n" + infoLog);
    }

    // Detach & delete once linked
    for (auto shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
    return program;
}

std::string BaseShader::ReadFile(const std::filesystem::path& filepath) const
{
    if (!std::filesystem::exists(filepath)) {
        Logger::GetLogger()->error("Shader file does not exist: {}", filepath.string());
        throw std::runtime_error("Shader file does not exist: " + filepath.string());
    }

    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        Logger::GetLogger()->error("Failed to open shader file: {}", filepath.string());
        throw std::runtime_error("Failed to open shader file: " + filepath.string());
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string BaseShader::ResolveIncludes(const std::string& source,
    const std::filesystem::path& directory,
    std::unordered_set<std::string>& includedFiles) const
{
    std::istringstream stream(source);
    std::ostringstream output;
    std::string line;
    bool inBlockComment = false;

    while (std::getline(stream, line)) {
        std::string originalLine = line;
        std::string trimmed = line;

        // Handle block comments
        if (inBlockComment) {
            size_t endPos = trimmed.find("*/");
            if (endPos != std::string::npos) {
                inBlockComment = false;
                trimmed = trimmed.substr(endPos + 2);
            }
            else {
                // Entire line is in block comment
                continue;
            }
        }

        // Check start of block comment
        size_t startBlock = trimmed.find("/*");
        if (startBlock != std::string::npos) {
            inBlockComment = true;
            trimmed = trimmed.substr(0, startBlock);
        }

        // Strip single-line comments
        size_t commentPos = trimmed.find("//");
        if (commentPos != std::string::npos) {
            trimmed = trimmed.substr(0, commentPos);
        }

        // Trim trailing whitespace
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t')) {
            trimmed.pop_back();
        }

        // Process #include
        const std::string includeDirective = "#include";
        if (trimmed.find(includeDirective) != std::string::npos) {
            // Example: #include "myfile.glsl"
            size_t startQuote = trimmed.find_first_of("\"<");
            size_t endQuote = trimmed.find_first_of("\">", startQuote + 1);
            if (startQuote == std::string::npos || endQuote == std::string::npos) {
                Logger::GetLogger()->error("Invalid #include syntax: {}", originalLine);
                throw std::runtime_error("Invalid #include syntax: " + originalLine);
            }

            std::string includeRelPath = trimmed.substr(startQuote + 1, endQuote - (startQuote + 1));
            auto includePath = (directory / includeRelPath).lexically_normal();

            // If not already included
            if (!includedFiles.count(includePath.string())) {
                includedFiles.insert(includePath.string());
                std::string includedSource = ReadFile(includePath);
                includedSource = ResolveIncludes(includedSource, includePath.parent_path(), includedFiles);
                output << includedSource << "\n";
            }
        }
        else {
            output << originalLine << "\n";
        }
    }

    return output.str();
}

bool BaseShader::LoadBinary()
{
    if (m_BinaryPath.empty()) { // Ensure m_BinaryPath is std::filesystem::path
        return false;
    }
    if (!std::filesystem::exists(m_BinaryPath)) {
        Logger::GetLogger()->warn("Shader binary file does not exist: {}", m_BinaryPath.string());
        return false;
    }

    std::ifstream inStream(m_BinaryPath, std::ios::binary);
    if (!inStream.good()) {
        Logger::GetLogger()->error("Failed to open shader binary file: {}", m_BinaryPath.string());
        return false;
    }

    // Read format
    GLenum binaryFormat = 0;
    inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));

    // Read binary data
    std::vector<GLubyte> binaryData{
        std::istreambuf_iterator<char>(inStream),
        std::istreambuf_iterator<char>()
    };

    if (binaryData.empty()) {
        Logger::GetLogger()->error("Shader binary file is empty: {}", m_BinaryPath.string());
        return false;
    }

    *m_RendererIDPtr = glCreateProgram();
    glProgramBinary(*m_RendererIDPtr, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));

    GLint success = GL_FALSE;
    glGetProgramiv(*m_RendererIDPtr, GL_LINK_STATUS, &success);
    if (!success) {
        GLint maxLen = 0;
        glGetProgramiv(*m_RendererIDPtr, GL_INFO_LOG_LENGTH, &maxLen);

        std::string infoLog(static_cast<size_t>(maxLen), '\0');
        glGetProgramInfoLog(*m_RendererIDPtr, maxLen, &maxLen, infoLog.data());

        glDeleteProgram(*m_RendererIDPtr);
        *m_RendererIDPtr = 0;
        Logger::GetLogger()->error("Failed to load shader binary '{}':\n{}", m_BinaryPath.string(), infoLog);
        return false;
    }

    Logger::GetLogger()->info("Successfully loaded shader binary from '{}'.", m_BinaryPath.string());
    return true;
}

void BaseShader::SaveBinary() const
{
    if (m_BinaryPath.empty()) {
        return;
    }
    GLint numFormats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
    if (numFormats == 0) {
        Logger::GetLogger()->error("Driver does not support program binary formats.");
        return;
    }

    GLint binaryLength = 0;
    glGetProgramiv(*m_RendererIDPtr, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    if (binaryLength <= 0) {
        Logger::GetLogger()->warn("Shader program has no binary length. Not saving to '{}'.", m_BinaryPath.string());
        return;
    }

    std::vector<GLubyte> binary(static_cast<size_t>(binaryLength));
    GLenum binaryFormat = 0;
    glGetProgramBinary(*m_RendererIDPtr, binaryLength, nullptr, &binaryFormat, binary.data());

    std::ofstream outFile(m_BinaryPath, std::ios::binary);
    if (!outFile.is_open()) {
        Logger::GetLogger()->error("Failed to open '{}' for writing shader binary.", m_BinaryPath.string());
        return;
    }

    outFile.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
    outFile.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    Logger::GetLogger()->info("Shader binary saved to '{}'.", m_BinaryPath.string());
}

// ----------------------------------------------------------------------
// Uniform Setters
// ----------------------------------------------------------------------
void BaseShader::SetUniform(std::string_view name, float value) const {
    glProgramUniform1f(*m_RendererIDPtr, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, int value) const {
    glProgramUniform1i(*m_RendererIDPtr, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, unsigned int value) const {
    glProgramUniform1ui(*m_RendererIDPtr, GetUniformLocation(name), value);
}

void BaseShader::SetUniform(std::string_view name, const glm::vec2& value) const {
    glProgramUniform2fv(*m_RendererIDPtr, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::vec3& value) const {
    glProgramUniform3fv(*m_RendererIDPtr, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::vec4& value) const {
    glProgramUniform4fv(*m_RendererIDPtr, GetUniformLocation(name), 1, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::mat3& value) const {
    glProgramUniformMatrix3fv(*m_RendererIDPtr, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void BaseShader::SetUniform(std::string_view name, const glm::mat4& value) const {
    glProgramUniformMatrix4fv(*m_RendererIDPtr, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}