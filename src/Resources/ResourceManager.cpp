#include "ResourceManager.h"

#include "Cube.h"

#include <unordered_map>


std::unordered_map<std::string, std::string> G_Texture_Path
{
	{"cuteDog", "../assets/cute_dog.png"},
	{"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"}
};

std::shared_ptr<Texture> ResourceManager::GetTexture(const std::string& textureName)
{
	if (textureName.empty())
		return nullptr;

	if (m_Textures.find(textureName) != m_Textures.end()) {
		return m_Textures[textureName];
	}

	std::shared_ptr<Texture> texture;
	if (G_Texture_Path.find(textureName) != G_Texture_Path.end())
		texture = std::make_shared<Texture>(G_Texture_Path[textureName]);
	else
		texture = std::make_shared<Texture>(textureName);

	m_Textures[textureName] = texture;
	return texture;
}

bool ResourceManager::DeleteTexture(const std::string& textureName)
{
	return m_Textures.erase(textureName);
}


std::shared_ptr<Mesh> CreateMesh(std::string meshName) {
	if (meshName == "cube")
		return std::make_shared<Cube>();
	else
		return nullptr;
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(const std::string& meshName)
{
	if (m_Meshes.find(meshName) != m_Meshes.end()) {
		return m_Meshes[meshName];
	}

	return CreateMesh(meshName);
}

bool ResourceManager::DeleteMesh(const std::string& meshName)
{
	return m_Meshes.erase(meshName);
}

std::unordered_map<std::string, std::string> G_Model_Path
{
	{"duck", "../assets/rubber_duck/scene.gltf"},
	{"pig", "../assets/pig_triangulated.obj"}
};

std::shared_ptr<Model> ResourceManager::GetModel(const std::string& modelName)
{
	if (m_Models.find(modelName) != m_Models.end()) {
		return m_Models[modelName];
	}

	std::shared_ptr<Model> model;
	if (G_Model_Path.find(modelName) != G_Model_Path.end())
		model = std::make_shared<Model>(G_Model_Path[modelName]);
	else
		model = std::make_shared<Model>(modelName);

	m_Models[modelName] = model;
	return model;
}

bool ResourceManager::DeleteModel(const std::string& modelName)
{
	return m_Models.erase(modelName);
}

std::unordered_map<std::string, std::string> G_Shader_Path
{
	{"basic", "../shaders/Basic.shader"},
	{"duck", "../shaders/Duck.shader"},
	{"lights", "../shaders/Lights.shader" },
	{"instance", "../shaders/Instance.shader" },
	{"pbr", "../shaders/PBR.shader" },
	{"brdf_compute", "../shaders/BRDFCompute.shader"},
	{"quad", "../shaders/Quad.shader"},
};

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& shaderName)
{
	if (m_Shaders.find(shaderName) != m_Shaders.end()) {
		return m_Shaders[shaderName];
	}

	std::shared_ptr<Shader> shader;
	if (G_Shader_Path.find(shaderName) != G_Shader_Path.end())
		shader = std::make_shared<Shader>(G_Shader_Path[shaderName]);
	else
		shader = std::make_shared<Shader>(shaderName);

	m_Shaders[shaderName] = shader;
	return shader;
}

bool ResourceManager::DeleteShader(const std::string& shaderName)
{
	return m_Shaders.erase(shaderName);
}

std::shared_ptr<ComputeShader> ResourceManager::GetComputeShader(const std::string& shaderName)
{
	if (m_ComputeShaders.find(shaderName) != m_ComputeShaders.end()) {
		return m_ComputeShaders[shaderName];
	}

	std::shared_ptr<ComputeShader> shader;
	if (G_Shader_Path.find(shaderName) != G_Shader_Path.end())
		shader = std::make_shared<ComputeShader>(G_Shader_Path[shaderName]);
	else
		shader = std::make_shared<ComputeShader>(shaderName);

	m_ComputeShaders[shaderName] = shader;
	return shader;
}

bool ResourceManager::DeleteComputeShader(const std::string& shaderName)
{
	return m_ComputeShaders.erase(shaderName);
}

std::shared_ptr<MeshBuffer> ResourceManager::GetMeshBuffer(const std::string& meshName, const MeshLayout& layout)
{
	MeshKey key = { meshName, layout };

	if (m_MeshBuffers.find(key) != m_MeshBuffers.end())
		return m_MeshBuffers[key];


	auto mesh = GetMesh(meshName);

	m_MeshBuffers[key] = std::make_shared<MeshBuffer>(mesh, layout);
	return m_MeshBuffers[key];

}

bool ResourceManager::DeleteMeshBuffer(const std::string& meshName, MeshLayout layout)
{
	MeshKey key = { meshName, layout };
	return m_MeshBuffers.erase(key);
}
