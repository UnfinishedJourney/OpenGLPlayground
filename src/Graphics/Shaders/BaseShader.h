#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <glm/glm.hpp>

class ShaderManager;

/// <future work>
/// 1) hot reloading
/// 2) vertex and fragment separate?
/// 3) bins to bin folder
/// 4) Shaderc, SPIR-V
/// </future work>
class BaseShader 
{
    friend class ShaderManager;
public:
    explicit BaseShader(const std::filesystem::path& filepath);
    virtual ~BaseShader();
    virtual void ReloadShader() = 0;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, unsigned int value) const;
    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;
    void SetUniform(const std::string& name, const glm::mat3& value) const;
    void SetUniform(const std::string& name, const glm::mat4& value) const;
    void SetUniform(const std::string& name, float v0, float v1, float v2) const;
    void SetUniform(const std::string& name, float v0, float v1, float v2, float v3) const;

protected:
    std::filesystem::path m_Filepath;
    unsigned int m_RendererID = 0;
    mutable std::unordered_map<std::string, int> m_UniformLocationCache;

    virtual void LoadShader(const std::filesystem::path& filepath) = 0;

    unsigned int CompileShader(unsigned int type, const std::string& source) const;
    unsigned int LinkProgram(const std::vector<unsigned int>& shaders) const;
    int GetUniformLocation(const std::string& name) const;
    std::string ReadFile(const std::filesystem::path& filepath) const;
    std::string ResolveIncludes(const std::string& source, const std::filesystem::path& directory) const;
    bool LoadBinary(const std::filesystem::path& binaryPath);
    void SaveBinary(const std::filesystem::path& binaryPath) const;
    void Bind() const;
    void Unbind() const;
};