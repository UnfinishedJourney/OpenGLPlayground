#include "BaseShader.h"
#include "ShaderUtils.h"
#include "Utilities/Logger.h"
#include <sstream>
#include <stdexcept>
#include <iterator>
#include <glm/gtc/type_ptr.hpp>

namespace Graphics {

    BaseShader::BaseShader(const std::filesystem::path& binaryPath)
        : m_BinaryPath(binaryPath), m_RendererID(0)
    {
    }

    BaseShader::~BaseShader() {
        if (m_RendererID != 0) {
            glDeleteProgram(m_RendererID);
        }
    }

    BaseShader::BaseShader(BaseShader&& other) noexcept
        : m_RendererID(other.m_RendererID),
        m_BinaryPath(std::move(other.m_BinaryPath)),
        m_UniformLocationCache(std::move(other.m_UniformLocationCache))
    {
        other.m_RendererID = 0;
    }

    BaseShader& BaseShader::operator=(BaseShader&& other) noexcept {
        if (this != &other) {
            if (m_RendererID != 0) {
                glDeleteProgram(m_RendererID);
            }
            m_RendererID = other.m_RendererID;
            m_BinaryPath = std::move(other.m_BinaryPath);
            m_UniformLocationCache = std::move(other.m_UniformLocationCache);
            other.m_RendererID = 0;
        }
        return *this;
    }

    void BaseShader::Bind() const {
        glUseProgram(m_RendererID);
    }

    void BaseShader::Unbind() const {
        glUseProgram(0);
    }

    GLint BaseShader::GetUniformLocation(std::string_view name) const {
        std::string uniformName(name);
        if (auto it = m_UniformLocationCache.find(uniformName); it != m_UniformLocationCache.end()) {
            return it->second;
        }
        GLint location = glGetUniformLocation(m_RendererID, uniformName.c_str());
        if (location == -1) {
            Logger::GetLogger()->warn("Uniform '{}' not found in shader program ID {}.", uniformName, m_RendererID);
        }
        m_UniformLocationCache[uniformName] = location;
        return location;
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

    GLuint BaseShader::CompileShader(GLenum shaderType, const std::string& source) const {
        GLuint shader = glCreateShader(shaderType);
        const char* srcPtr = source.c_str();
        glShaderSource(shader, 1, &srcPtr, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            std::string infoLog(static_cast<size_t>(infoLen), '\0');
            glGetShaderInfoLog(shader, infoLen, &infoLen, infoLog.data());
            glDeleteShader(shader);

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

    GLuint BaseShader::LinkProgram(const std::vector<GLuint>& shaders) const {
        GLuint program = glCreateProgram();
        for (auto shader : shaders) {
            glAttachShader(program, shader);
        }
        glLinkProgram(program);

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
        for (auto shader : shaders) {
            glDetachShader(program, shader);
            glDeleteShader(shader);
        }
        return program;
    }

    std::string BaseShader::ReadFile(const std::filesystem::path& filepath) const {
        return ShaderUtils::ReadFile(filepath);
    }

    std::string BaseShader::ResolveIncludes(const std::string& source,
        const std::filesystem::path& directory,
        std::unordered_set<std::string>& includedFiles) const {
        return ShaderUtils::ResolveIncludes(source, directory, includedFiles);
    }

    bool BaseShader::LoadBinary() {
        if (m_BinaryPath.empty())
            return false;
        if (!std::filesystem::exists(m_BinaryPath)) {
            Logger::GetLogger()->warn("Shader binary file does not exist: {}", m_BinaryPath.string());
            return false;
        }
        std::ifstream inStream(m_BinaryPath, std::ios::binary);
        if (!inStream.good()) {
            Logger::GetLogger()->error("Failed to open shader binary file: {}", m_BinaryPath.string());
            return false;
        }
        GLenum binaryFormat = 0;
        inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));
        std::vector<GLubyte> binaryData{ std::istreambuf_iterator<char>(inStream),
                                          std::istreambuf_iterator<char>() };
        if (binaryData.empty()) {
            Logger::GetLogger()->error("Shader binary file is empty: {}", m_BinaryPath.string());
            return false;
        }
        m_RendererID = glCreateProgram();
        glProgramBinary(m_RendererID, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));
        GLint success = GL_FALSE;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        if (!success) {
            GLint maxLen = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLen);
            std::string infoLog(static_cast<size_t>(maxLen), '\0');
            glGetProgramInfoLog(m_RendererID, maxLen, &maxLen, infoLog.data());
            glDeleteProgram(m_RendererID);
            m_RendererID = 0;
            Logger::GetLogger()->error("Failed to load shader binary '{}':\n{}", m_BinaryPath.string(), infoLog);
            return false;
        }
        Logger::GetLogger()->info("Successfully loaded shader binary from '{}'.", m_BinaryPath.string());
        return true;
    }

    void BaseShader::SaveBinary() const {
        if (m_BinaryPath.empty())
            return;
        GLint numFormats = 0;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
        if (numFormats == 0) {
            Logger::GetLogger()->error("Driver does not support program binary formats.");
            return;
        }
        GLint binaryLength = 0;
        glGetProgramiv(m_RendererID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        if (binaryLength <= 0) {
            Logger::GetLogger()->warn("Shader program has no binary length. Not saving to '{}'.", m_BinaryPath.string());
            return;
        }
        std::vector<GLubyte> binary(static_cast<size_t>(binaryLength));
        GLenum binaryFormat = 0;
        glGetProgramBinary(m_RendererID, binaryLength, nullptr, &binaryFormat, binary.data());
        std::ofstream outFile(m_BinaryPath, std::ios::binary);
        if (!outFile.is_open()) {
            Logger::GetLogger()->error("Failed to open '{}' for writing shader binary.", m_BinaryPath.string());
            return;
        }
        outFile.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
        outFile.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        Logger::GetLogger()->info("Shader binary saved to '{}'.", m_BinaryPath.string());
    }

} // namespace Graphics