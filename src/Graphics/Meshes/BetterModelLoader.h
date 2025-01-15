#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include <assimp/scene.h>       
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialParamType.h"
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Textures/TextureData.h"
#include "Scene/SceneGraph.h"
#include "Utilities/Logger.h"
#include "Resources/MaterialManager.h"
#include "Resources/TextureManager.h"

#include "MeshInfo.h"

/**
 * @brief Container for a single mesh plus the associated material name.
 */
struct BetterModelMeshData
{
    std::shared_ptr<Mesh> mesh;
    std::string           materialName;
};

/**
 * @brief Holds all final data loaded by BetterModelLoader: the meshes and any created materials.
 */
struct BetterModelData
{
    std::vector<BetterModelMeshData> meshesData;
    std::vector<std::string>         createdMaterials;
};

/**
 * @brief Holds textures loaded for a single mesh, categorized by TextureType.
 */
struct BetterMeshTextures
{
    std::unordered_map<TextureType, std::shared_ptr<ITexture>> textures;
};

class BetterModelLoader
{
public:
    BetterModelLoader();
    ~BetterModelLoader();

    bool LoadModel(const std::string& filePath,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        SceneGraph& sceneGraph,
        bool centerModel);

    const BetterModelData& GetModelData() const { return m_Data; }

    static std::string GetModelPath(const std::string& modelName);

private:
    BetterModelData m_Data;
    int m_FallbackMaterialCounter = 0;

private:
    void LoadSceneMaterials(const aiScene* scene,
        const MaterialLayout& matLayout,
        const std::string& directory);

    std::string CreateMaterialForAssimpMat(const aiMaterial* aiMat,
        const MaterialLayout& matLayout,
        const std::string& directory);

    std::string EnsureUniqueMaterialName(const std::string& baseName);
    void LoadMaterialProperties(const aiMaterial* aiMat,
        std::shared_ptr<Material> mat,
        const MaterialLayout& matLayout);
    void LoadMaterialTextures(const aiMaterial* aiMat,
        std::shared_ptr<Material> material,
        const MaterialLayout& matLayout,
        const std::string& directory);

    /**
     * @brief Actually loads the texture files from disk, returning a map of TextureType -> ITexture
     */
    BetterMeshTextures LoadMeshTextures(const aiMaterial* material,
        const std::string& directory);

    std::string CreateFallbackMaterialName();
    std::shared_ptr<Material> createFallbackMaterial(const std::string& name,
        const MaterialLayout& matLayout);

    void ProcessAssimpNode(const aiScene* scene,
        const aiNode* ainode,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        const std::string& directory,
        SceneGraph& sceneGraph,
        int parentNode = -1);

    void ProcessAssimpMesh(const aiScene* scene,
        const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        int meshIndex,
        const std::string& directory);

    void GenerateLODs(const std::vector<uint32_t>& srcIndices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const;

    void SetNodeBoundingVolumes(const aiScene* scene,
        const aiNode* ainode,
        int currentSGNode,
        SceneGraph& sceneGraph);

    void CenterScene(SceneGraph& sceneGraph);

    glm::mat4 AiToGlm(const aiMatrix4x4& m);
};