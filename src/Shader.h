#pragma once

#include <string>
#include <unordered_map>

#include "glm.hpp"

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform(const std::string& name, float value) const;
	void SetUniform(const std::string& name, const glm::vec3& value) const;
	void SetUniform(const std::string& name, float v0, float v1, float v2) const;
	void SetUniform(const std::string& name, const glm::vec4& value) const;
	void SetUniform(const std::string& name, float v0, float v1, float v2, float v3) const;
	void SetUniform(const std::string& name, const glm::mat3& value) const;
	void SetUniform(const std::string& name, const glm::mat4& value) const;

	bool GetStatus() const
	{
		return m_IsBound;
	}

private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	mutable bool m_IsBound = false;
	unsigned int CompileShader(unsigned int type, const std::string& source);
	int GetUniformLocation(const std::string& name) const;
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	ShaderProgramSource ParseShader(const std::string& filepath);

};