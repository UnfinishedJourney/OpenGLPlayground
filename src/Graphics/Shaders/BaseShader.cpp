#include "BaseShader.h"
#include "ShaderUtils.h"
#include "Utilities/Logger.h"
#include <sstream>
#include <stdexcept>
#include <iterator>
#include <glm/gtc/type_ptr.hpp>

namespace graphics {

    BaseShader::BaseShader(const std::filesystem::path& binaryPath)
        : binaryPath_(binaryPath), rendererId_(0)
    {
    }

    BaseShader::~BaseShader() {
        if (rendererId_ != 0) {
            glDeleteProgram(rendererId_);
        }
    }

    BaseShader::BaseShader(BaseShader&& other) noexcept
        : rendererId_(other.rendererId_),
        binaryPath_(std::move(other.binaryPath_)),
        uniformLocationCache_(std::move(other.uniformLocationCache_))
    {
        other.rendererId_ = 0;
    }

    BaseShader& BaseShader::operator=(BaseShader&& other) noexcept {
        if (this != &other) {
            if (rendererId_ != 0) {
                glDeleteProgram(rendererId_);
            }
            rendererId_ = other.rendererId_;
            binaryPath_ = std::move(other.binaryPath_);
            uniformLocationCache_ = std::move(other.uniformLocationCache_);
            other.rendererId_ = 0;
        }
        return *this;
    }

    void BaseShader::Bind() const {
        glUseProgram(rendererId_);
    }

    void BaseShader::Unbind() const {
        glUseProgram(0);
    }

    GLint BaseShader::GetUniformLocation(std::string_view name) const {
        std::string uniformName(name);
        if (auto it = uniformLocationCache_.find(uniformName); it != uniformLocationCache_.end())
            return it->second;
        GLint location = glGetUniformLocation(rendererId_, uniformName.c_str());
        if (location == -1) {
            Logger::GetLogger()->warn("Uniform '{}' not found in shader program ID {}.", uniformName, rendererId_);
        }
        uniformLocationCache_[uniformName] = location;
        return location;
    }

    void BaseShader::SetUniform(std::string_view name, float value) const {
        glProgramUniform1f(rendererId_, GetUniformLocation(name), value);
    }

    void BaseShader::SetUniform(std::string_view name, int value) const {
        glProgramUniform1i(rendererId_, GetUniformLocation(name), value);
    }

    void BaseShader::SetUniform(std::string_view name, unsigned int value) const {
        glProgramUniform1ui(rendererId_, GetUniformLocation(name), value);
    }

    void BaseShader::SetUniform(std::string_view name, const glm::vec2& value) const {
        glProgramUniform2fv(rendererId_, GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void BaseShader::SetUniform(std::string_view name, const glm::vec3& value) const {
        glProgramUniform3fv(rendererId_, GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void BaseShader::SetUniform(std::string_view name, const glm::vec4& value) const {
        glProgramUniform4fv(rendererId_, GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void BaseShader::SetUniform(std::string_view name, const glm::mat3& value) const {
        glProgramUniformMatrix3fv(rendererId_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void BaseShader::SetUniform(std::string_view name, const glm::mat4& value) const {
        glProgramUniformMatrix4fv(rendererId_, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
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
        for (auto shader : shaders)
            glAttachShader(program, shader);
        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            std::string infoLog(static_cast<size_t>(infoLen), '\0');
            glGetProgramInfoLog(program, infoLen, &infoLen, infoLog.data());

            glDeleteProgram(program);
            for (auto shader : shaders)
                glDeleteShader(shader);

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
        return shaderUtils::ReadFile(filepath);
    }

    std::string BaseShader::ResolveIncludes(const std::string& source,
        const std::filesystem::path& directory,
        std::unordered_set<std::string>& includedFiles) const {
        return shaderUtils::ResolveIncludes(source, directory, includedFiles);
    }

    bool BaseShader::LoadBinary() {
        if (binaryPath_.empty())
            return false;
        if (!std::filesystem::exists(binaryPath_)) {
            Logger::GetLogger()->warn("Shader binary file does not exist: {}", binaryPath_.string());
            return false;
        }
        std::ifstream inStream(binaryPath_, std::ios::binary);
        if (!inStream.good()) {
            Logger::GetLogger()->error("Failed to open shader binary file: {}", binaryPath_.string());
            return false;
        }
        GLenum binaryFormat = 0;
        inStream.read(reinterpret_cast<char*>(&binaryFormat), sizeof(GLenum));
        std::vector<GLubyte> binaryData{ std::istreambuf_iterator<char>(inStream),
                                          std::istreambuf_iterator<char>() };
        if (binaryData.empty()) {
            Logger::GetLogger()->error("Shader binary file is empty: {}", binaryPath_.string());
            return false;
        }
        rendererId_ = glCreateProgram();
        glProgramBinary(rendererId_, binaryFormat, binaryData.data(), static_cast<GLsizei>(binaryData.size()));
        GLint success = GL_FALSE;
        glGetProgramiv(rendererId_, GL_LINK_STATUS, &success);
        if (!success) {
            GLint maxLen = 0;
            glGetProgramiv(rendererId_, GL_INFO_LOG_LENGTH, &maxLen);
            std::string infoLog(static_cast<size_t>(maxLen), '\0');
            glGetProgramInfoLog(rendererId_, maxLen, &maxLen, infoLog.data());
            glDeleteProgram(rendererId_);
            rendererId_ = 0;
            Logger::GetLogger()->error("Failed to load shader binary '{}':\n{}", binaryPath_.string(), infoLog);
            return false;
        }
        Logger::GetLogger()->info("Successfully loaded shader binary from '{}'.", binaryPath_.string());
        return true;
    }

    void BaseShader::SaveBinary() const {
        if (binaryPath_.empty())
            return;
        GLint numFormats = 0;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
        if (numFormats == 0) {
            Logger::GetLogger()->error("Driver does not support program binary formats.");
            return;
        }
        GLint binaryLength = 0;
        glGetProgramiv(rendererId_, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        if (binaryLength <= 0) {
            Logger::GetLogger()->warn("Shader program has no binary length. Not saving to '{}'.", binaryPath_.string());
            return;
        }
        std::vector<GLubyte> binary(static_cast<size_t>(binaryLength));
        GLenum binaryFormat = 0;
        glGetProgramBinary(rendererId_, binaryLength, nullptr, &binaryFormat, binary.data());
        std::ofstream outFile(binaryPath_, std::ios::binary);
        if (!outFile.is_open()) {
            Logger::GetLogger()->error("Failed to open '{}' for writing shader binary.", binaryPath_.string());
            return;
        }
        outFile.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(GLenum));
        outFile.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        Logger::GetLogger()->info("Shader binary saved to '{}'.", binaryPath_.string());
    }

} // namespace graphics