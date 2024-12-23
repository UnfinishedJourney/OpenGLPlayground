#pragma once
#include <string>
#include <vector>
#include "Scene/SceneGraph.h"
#include "MeshLayout.h"
#include "Mesh.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Model.h"
#include <filesystem>

class ModelLoader {
public:
    bool LoadIntoSceneGraph(
        const Model& model,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        SceneGraph& sceneGraph,
        std::vector<MeshInfo>& outMeshes,
        std::vector<std::string>& outMaterials);
    static std::string GetModelPath(const std::string& modelName);
};

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "MeshLayout.h"
#include "Mesh.h"      
#include "Utilities/Logger.h"

//need to think about pointers
struct ModelData
{
    MeshLayout m_MeshLayout;
    MaterialLayout m_MatLayout;
    std::vector<TextureData> m_TexData; // Placeholder for future texture handling
    std::vector<glm::mat4> m_LocalTransforms;
    std::vector<std::string> m_Materials;
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
};

struct ModelNode
{
    int localTransform_ = -1;          // Index to local transforms; -1 indicates uninitialized
    std::vector<int> meshIndices;       // Indices to meshes; empty vector means no meshes
    int parent_ = -1;                   // -1 indicates no parent (root node)
    int firstChild_ = -1;               // -1 indicates no first child
    int nextSibling_ = -1;              // -1 indicates no next sibling
    int lastSibling_ = -1;              // -1 indicates no last sibling
    int level_ = 0;                     // Hierarchical level in the scene graph
};

class ModelLoader2 {
public:
    bool LoadModel(
        const std::string& modelName,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout);

    static std::string GetModelPath(const std::string& modelName);

private:
    std::vector<ModelNode> m_Nodes;
    ModelData m_ModelData;

    void CreateMaterial(const MaterialLayout& matLayout, const aiMaterial& aiMat);
    void ProcessNode(const aiScene* scene, const aiNode* node, int parentIndex = -1);
    Mesh ProcessMesh(const aiScene* scene, const aiMesh* aiMesh);
    void EnsureUnknownMaterialExists();
};