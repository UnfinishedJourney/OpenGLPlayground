#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Mesh.h"
#include "MeshLayout.h"
#include "Graphics/Textures/TextureData.h"

struct MeshTextures {
    std::unordered_map<TextureType, TextureData> textureData;
};

struct MeshInfo {
    MeshTextures meshTextures;
    std::shared_ptr<Mesh> mesh;
    int materialIndex = -1;
};

struct MaterialInfo {
    std::string name;
    // Possibly store texture file paths or additional parameters
};

class Model {
public:
    Model(const std::string& pathToModel, bool centerModel, const MeshLayout& requestedLayout);
    const std::vector<MeshInfo>& GetMeshesInfo() const { return m_MeshInfos; }
    const std::vector<MaterialInfo>& GetMaterials() const { return m_Materials; }

    struct Node {
        std::string name;
        int parent = -1;
        std::vector<int> children;
        std::vector<int> meshes; // indices into m_MeshInfos
        glm::mat4 localTransform;
    };
    const std::vector<Node>& GetNodes() const { return m_Nodes; }

private:
    friend class ModelLoader;
    void LoadFromAssimp(const MeshLayout& layout, bool centerModel);
    void ProcessNode(const struct aiScene* scene, const struct aiNode* ainode, int parentIndex);
    int AddNode(const std::string& name, int parent, const glm::mat4& localTransform);
    void ProcessMesh(const struct aiScene* scene, const struct aiMesh* aiMesh, const MeshLayout& layout);
    void ProcessLODs(std::vector<uint32_t>& indices, const std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods);
    MeshTextures LoadMeshTextures(const struct aiMaterial* material, const std::string& directory);

    std::string m_FilePath;
    std::vector<MeshInfo> m_MeshInfos;
    std::vector<MaterialInfo> m_Materials;
    std::vector<Node> m_Nodes;
};