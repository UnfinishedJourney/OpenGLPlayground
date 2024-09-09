#pragma once

#include "Utility.h"
#include <string>
#include <vector>
#include <memory>
#include "glm.hpp"
#include "Texture.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh {
public:
	Mesh()
	{}
    Mesh(std::unique_ptr<VertexArray> vao, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib, std::unique_ptr<Texture> tex);
	virtual ~Mesh()
	{}
    virtual void Draw(const std::unique_ptr<Shader>& shader);
	virtual void Update(float dt);
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&& other) noexcept {
		m_VAO = std::move(other.m_VAO);
		m_VB = std::move(other.m_VB);
		m_IB = std::move(other.m_IB);
		m_Texture = std::move(other.m_Texture);
		m_Model = other.m_Model;
	}

	Mesh& operator=(Mesh&& other) noexcept {
		if (this != &other) {
			m_VAO = std::move(other.m_VAO);
			m_VB = std::move(other.m_VB);
			m_IB = std::move(other.m_IB);
			m_Texture = std::move(other.m_Texture);
			m_Model = other.m_Model;
		}
		return *this;
	}

	void LeftMatrixMultiplier(glm::mat4 multiplier) {
		m_Model = multiplier * m_Model;
	}

	void RightMatrixMultiplier(glm::mat4 multiplier) {
		m_Model = m_Model * multiplier;
	}

protected:
	// mesh data
	std::unique_ptr<VertexArray> m_VAO;
	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<IndexBuffer> m_IB;
	std::unique_ptr<Texture> m_Texture;

	glm::mat4 m_Model;
};

class Model
{
private:
	std::string m_FilePath;
	std::unique_ptr<Shader> m_Shader;
	std::unique_ptr<Mesh> m_Mesh;
	std::vector<Mesh> m_Meshes;
public:
	void Draw();
	void Update(float dt);
	Model(const std::string& path_to_model, const std::string& path_to_shader);
	void ProcessModel();
	void AddShader(std::unique_ptr<Shader> shader)
	{
		m_Shader = std::move(shader);
	}
	void ProcessNode(const aiScene* scene, const aiNode* node);
	void ProcessMesh(const aiScene* scene, const aiMesh* mesh);

	~Model();
};