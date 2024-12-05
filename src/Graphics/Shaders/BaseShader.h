#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "ShaderDeleter.h"

class BaseShader {
public:
    explicit BaseShader(const std::filesystem::path& binaryPath = "");
    virtual ~BaseShader() = default;

    BaseShader(const BaseShader&) = delete;
    BaseShader& operator=(const BaseShader&) = delete;
    BaseShader(BaseShader&& other) noexcept = default;
    BaseShader& operator=(BaseShader&& other) noexcept = default;

    virtual void ReloadShader() = 0;

    void Bind() const;
    void Unbind() const;

    void SetUniform(std::string_view name, float value) const;
    void SetUniform(std::string_view name, int value) const;
    void SetUniform(std::string_view name, unsigned int value) const;
    void SetUniform(std::string_view name, const glm::vec2& value) const;
    void SetUniform(std::string_view name, const glm::vec3& value) const;
    void SetUniform(std::string_view name, const glm::vec4& value) const;
    void SetUniform(std::string_view name, const glm::mat3& value) const;
    void SetUniform(std::string_view name, const glm::mat4& value) const;

    bool LoadBinary();

protected:
    std::unique_ptr<GLuint, ShaderDeleter> m_RendererIDPtr = nullptr;
    GLenum m_Usage = GL_STATIC_DRAW;
    std::filesystem::path m_BinaryPath;

    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    GLuint CompileShader(GLenum shaderType, const std::string& source) const;
    GLuint LinkProgram(const std::vector<GLuint>& shaders) const;

    std::string ReadFile(const std::filesystem::path& filepath) const;
    std::string ResolveIncludes(const std::string& source,
        const std::filesystem::path& directory,
        std::unordered_set<std::string>& includedFiles) const;

    void SaveBinary() const;
    GLint GetUniformLocation(std::string_view name) const;
};