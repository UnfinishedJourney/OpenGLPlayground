#pragma once

#include <string>
#include <unordered_map>

#include "glm.hpp"

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
	std::string ComputeSource;
};

class BaseShader
{
public:
	BaseShader(const std::string& filepath);
	virtual ~BaseShader();
	void Bind() const;
	void Unbind() const;

	void SetUniform(const std::string& name, float value) const;
	void SetUniform(const std::string& name, int value) const;
	void SetUniform(const std::string& name, unsigned int value) const;
	void SetUniform(const std::string& name, const glm::vec3& value) const;
	void SetUniform(const std::string& name, float v0, float v1, float v2) const;
	void SetUniform(const std::string& name, const glm::vec4& value) const;
	void SetUniform(const std::string& name, float v0, float v1, float v2, float v3) const;
	void SetUniform(const std::string& name, const glm::mat3& value) const;
	void SetUniform(const std::string& name, const glm::mat4& value) const;


protected:
	std::string m_FilePath;
	unsigned int m_RendererID;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	virtual unsigned int CompileShader(unsigned int type, const std::string& source) = 0;
	int GetUniformLocation(const std::string& name) const;
	virtual ShaderProgramSource ParseShader(const std::string& filepath) = 0;
	std::string ParseIncludedShader(const std::string& includeFile);

};