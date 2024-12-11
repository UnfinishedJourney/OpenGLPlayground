#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Textures/OpenGLTexture.h"

struct MeshTextures {
    std::unordered_map<TextureType, std::shared_ptr<ITexture>> textures;
};

struct MeshInfo {
    MeshTextures meshTextures;
    std::shared_ptr<Mesh> mesh;
};

class Model {
public:
    explicit Model(const std::string& pathToModel, bool centerModel = true);

    size_t GetMeshCount() const { return m_MeshInfos.size(); }
    std::shared_ptr<MeshBuffer> GetMeshBuffer(size_t meshIndex, const MeshLayout& layout);
    std::shared_ptr<ITexture> GetTexture(size_t meshIndex, TextureType type) const;

    const std::vector<MeshInfo>& GetMeshesInfo() const { return m_MeshInfos; }
    std::vector<std::shared_ptr<MeshBuffer>> GetMeshBuffers(const MeshLayout& layout);

private:
    void LoadModel();
    void ProcessNode(const struct aiScene* scene, const struct aiNode* node);
    void ProcessMesh(const struct aiScene* scene, const struct aiMesh* mesh);
    void CenterModel();
    glm::vec3 CalculateModelCenter() const;
    MeshTextures LoadTextures(struct aiMaterial* material, const std::string& directory);

    // Generates LOD levels for the mesh indices
    void ProcessLODs(std::vector<uint32_t>& indices, const std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods);

    std::string m_FilePath;
    std::vector<MeshInfo> m_MeshInfos;
    std::vector<std::unordered_map<MeshLayout, std::shared_ptr<MeshBuffer>>> m_MeshBuffersCache;
};