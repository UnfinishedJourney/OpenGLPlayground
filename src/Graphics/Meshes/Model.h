#pragma once

#include "Utilities/Utility.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Buffers/MeshBuffer.h"

#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

struct MeshTextures {
    std::unordered_map<TextureType, std::shared_ptr<Texture2D>> textures;
};

struct MeshInfo {
    MeshTextures meshTextures;
    std::shared_ptr<Mesh> mesh;
};

class Model {
public:
    explicit Model(const std::string& pathToModel, bool centerModel = true);

    size_t GetMeshCount() const { return m_MeshesInfo.size(); }
    std::shared_ptr<MeshBuffer> GetMeshBuffer(size_t meshIndex, const MeshLayout& layout);
    std::shared_ptr<Texture2D> GetTexture(size_t meshIndex, TextureType type) const;

    const std::vector<MeshInfo>& GetMeshesInfo() const { return m_MeshesInfo; }
    std::vector<std::shared_ptr<MeshBuffer>> GetMeshBuffers(const MeshLayout& layout);

private:
    void ProcessModel();
    void ProcessNode(const aiScene* scene, const aiNode* node);
    void ProcessMesh(const aiScene* scene, const aiMesh* mesh);
    void CenterModel();
    glm::vec3 CalculateModelCenter() const;
    MeshTextures LoadTextures(const aiScene* scene, aiMaterial* aiMat, const std::string& directory);

    std::string m_FilePath;
    std::vector<MeshInfo> m_MeshesInfo;
    std::vector<std::unordered_map<MeshLayout, std::shared_ptr<MeshBuffer>>> m_MeshBuffersCache;
};