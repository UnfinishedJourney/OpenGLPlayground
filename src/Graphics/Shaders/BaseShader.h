#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_set>

#include <glad/glad.h>
#include <glm/glm.hpp>

class BaseShader {
public:
    BaseShader(std::filesystem::path sourcePath, std::filesystem::path binaryPath);
    virtual ~BaseShader();

    virtual void ReloadShader() = 0;

    void Bind() const;
    void Unbind() const;

    // Uniform setters
    void SetUniform(std::string_view name, float value) const;
    void SetUniform(std::string_view name, int value) const;
    void SetUniform(std::string_view name, unsigned int value) const;
    void SetUniform(std::string_view name, const glm::vec2& value) const;
    void SetUniform(std::string_view name, const glm::vec3& value) const;
    void SetUniform(std::string_view name, const glm::vec4& value) const;
    void SetUniform(std::string_view name, const glm::mat3& value) const;
    void SetUniform(std::string_view name, const glm::mat4& value) const;

protected:
    GLuint m_RendererID = 0;
    std::filesystem::path m_SourcePath;
    std::filesystem::path m_BinaryPath;

    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    // Compilation and linking
    GLuint CompileShader(GLenum shaderType, const std::string& source) const;
    GLuint LinkProgram(const std::vector<GLuint>& shaders) const;

    // Utility methods
    std::string ReadFile(const std::filesystem::path& filepath) const;
    std::string ResolveIncludes(const std::string& source, const std::filesystem::path& directory, std::unordered_set<std::string>& includedFiles) const;

    // Binary handling
    bool LoadBinary();
    void SaveBinary() const;

    GLint GetUniformLocation(std::string_view name) const;
};