#pragma once

#include "Utilities/Utility.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Meshes/Mesh.h"

#include "gtc/matrix_transform.hpp"
#include "glm.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Model
{
public:
	Model(const std::string& path_to_model, bool bCenterModel = true);

	size_t GetMeshCount() const { return m_Meshes.size(); }
	std::shared_ptr<MeshBuffer> GetMeshBuffer(size_t meshIndex, const MeshLayout& layout);
	std::vector<std::shared_ptr<MeshBuffer>> GetMeshBuffers(const MeshLayout& layout);

private:
	std::string m_FilePath;
	std::vector<std::shared_ptr<Mesh>> m_Meshes;
	std::vector<std::shared_ptr<Texture2D>> m_Textures;
	std::vector<std::unordered_map<MeshLayout, std::shared_ptr<MeshBuffer>>> m_MeshBuffersCache;
	void ProcessModel();
	void ProcessNode(const aiScene* scene, const aiNode* node);
	void ProcessMesh(const aiScene* scene, const aiMesh* mesh);
	glm::vec3 CalculateModelCenter() const;
	void CenterModel();
};