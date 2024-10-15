#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/Cube.h"

#include <unordered_map>

ResourceManager::ResourceManager()
{
	m_ShaderManager = std::make_unique<ShaderManager>("../shaders/metadata.json", "../shaders/config.json");
	m_MaterialManager = std::make_unique<MaterialManager>();
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& textureName)
{
	return m_MaterialManager->GetTexture(textureName);
}

std::shared_ptr<CubeMapTexture> ResourceManager::GetCubeMapTexture(const std::string& name)
{
	if (name.empty())
		return nullptr;

	auto it = m_TexturesCubeMap.find(name);
	if (it != m_TexturesCubeMap.end())
	{
		return it->second;
	}

	auto texIt = m_TextureCubeMap.find(name);
	if (texIt == m_TextureCubeMap.end())
	{
		std::cerr << "CubeMapTexture not found: " << name << std::endl;
		return nullptr;
	}

	const auto& facePaths = texIt->second;

	std::shared_ptr<CubeMapTexture> cubeMapTexture;
	try
	{
		cubeMapTexture = std::make_shared<CubeMapTexture>(facePaths);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to load CubeMapTexture '" << name << "': " << e.what() << std::endl;
		return nullptr;
	}

	m_TexturesCubeMap[name] = cubeMapTexture;
	return cubeMapTexture;
}

void ResourceManager::BindCubeMapTexture(const std::string& name, unsigned int slot) const
{
	if (m_CurrentlyBoundCubeMap == name)
		return;

	auto cubeMap = m_TexturesCubeMap.at(name);
	if (cubeMap) {
		cubeMap->Bind(slot);
	}
	else {
		throw std::runtime_error("CubeMapTexture '" + name + "' not found.");
	}
	m_CurrentlyBoundCubeMap = name;

}

void ResourceManager::SetUniform(const std::string& uniName, UniformValue uni)
{
	std::visit([&](auto&& arg) {
		m_ShaderManager->GetCurrentlyBoundShader()->SetUniform(uniName, arg);
		Logger::GetLogger()->debug("Set uniform '{}' with value.", uniName);
		}, uni);
}

std::shared_ptr<Mesh> CreateMesh(std::string meshName) {
	if (meshName == "cube")
		return std::make_shared<Cube>();
	else if (meshName == "torus")
		return nullptr;
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

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& materialName)
{
	return m_MaterialManager->GetMaterial(materialName);
}

std::unordered_map<std::string, std::string> G_Model_Path
{
	{"duck", "../assets/rubber_duck/scene.gltf"},
	{"damagedHelmet", "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"},
	{"pig", "../assets/pig_triangulated.obj"}
};

//std::shared_ptr<Model> ResourceManager::GetModel(const std::string& modelName)
//{
//	if (m_Models.find(modelName) != m_Models.end()) {
//		return m_Models[modelName];
//	}
//
//	std::shared_ptr<Model> model;
//	if (G_Model_Path.find(modelName) != G_Model_Path.end())
//		model = std::make_shared<Model>(G_Model_Path[modelName]);
//	else
//		model = std::make_shared<Model>(modelName);
//
//	m_Models[modelName] = model;
//	return model;
//}
//
//bool ResourceManager::DeleteModel(const std::string& modelName)
//{
//	return m_Models.erase(modelName);
//}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& shaderName)
{
	return m_ShaderManager->GetShader(shaderName);
}

std::shared_ptr<ComputeShader> ResourceManager::GetComputeShader(const std::string& shaderName)
{
	return m_ShaderManager->GetComputeShader(shaderName);
}

std::shared_ptr<MeshBuffer> ResourceManager::GetMeshBuffer(const std::string& meshName, const MeshLayout& mLayout)
{
	MeshKey key = { meshName, mLayout };

	if (m_MeshBuffers.find(key) != m_MeshBuffers.end())
		return m_MeshBuffers[key];


	auto mesh = GetMesh(meshName);

	m_MeshBuffers[key] = std::make_shared<MeshBuffer>(mesh, mLayout);
	return m_MeshBuffers[key];

}

bool ResourceManager::DeleteMeshBuffer(const std::string& meshName, MeshLayout mLayout)
{
	MeshKey key = { meshName, mLayout };
	return m_MeshBuffers.erase(key);
}
