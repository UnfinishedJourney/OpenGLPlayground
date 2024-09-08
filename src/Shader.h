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
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	//Set uniforms
	void SetUniformf(const std::string& name, float v) const;
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const;
private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	int GetUniformLocation(const std::string& name) const;
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	ShaderProgramSource ParseShader(const std::string& filepath);
};