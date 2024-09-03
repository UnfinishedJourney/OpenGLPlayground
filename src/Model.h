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

    Mesh(std::unique_ptr<VertexArray> vao, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib, std::unique_ptr<Texture> tex);
    void Draw(const std::unique_ptr<Shader>& shader);
private:
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
	Model(const std::string& path);
	void ProcessModel();
	void ProcessNode(const aiScene* scene, const aiNode* node);
	void ProcessMesh(const aiScene* scene, const aiMesh* mesh);
	~Model();
};