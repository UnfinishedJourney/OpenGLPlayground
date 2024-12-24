#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Include GLM extensions
#include <glm/gtc/matrix_transform.hpp> // Added for glm::translate

// Assimp Forward Declarations
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiNode;

// Forward declare your engine classes
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialParamType.h"
#include "Graphics/Textures/TextureType.h"
#include "Graphics/Textures/TextureData.h"
#include "Scene/SceneGraph.h"
#include "Utilities/Logger.h"
#include "Resources/MaterialManager.h"

/**
 * A container for the final loaded data:
 *  - The meshes
 *  - A parallel array of material names or indices
 */
struct BetterModelMeshData
{
    std::shared_ptr<Mesh> mesh;
    std::string           materialName;
};

/**
 * This struct holds the final data loaded by the loader:
 */
struct BetterModelData
{
    std::vector<BetterModelMeshData> meshesData;      ///< Each mesh + the associated material name
    std::vector<std::string>         createdMaterials; ///< All unique material names created
    // If you want node hierarchy, you can add it here (like in your older code).
};

class BetterModelLoader {
public:
    BetterModelLoader();
    ~BetterModelLoader();

    /**
     * Loads a model from the given file path into the provided SceneGraph.
     * @param filePath Path to the model file.
     * @param meshLayout Defines the mesh data to be loaded.
     * @param matLayout Defines the material data to be loaded.
     * @param centerModel If true, centers the model at the origin.
     * @param sceneGraph The SceneGraph to populate with nodes.
     * @return True if loading is successful, false otherwise.
     */
    bool LoadModel(
        const std::string& filePath,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        bool centerModel,
        SceneGraph& sceneGraph
    );

    const BetterModelData& GetModelData() {
        return m_Data;
    }

private:
    // Existing member variables
    BetterModelData m_Data;
    int m_FallbackMaterialCounter;

    // New private methods for scene graph integration
    void processAssimpNode(
        const aiScene* scene,
        const aiNode* ainode,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        const std::string& directory,
        SceneGraph& sceneGraph,
        int parentNode = -1
    );

    // Existing private methods
    void loadSceneMaterials(const aiScene* scene, const MaterialLayout& matLayout);
    std::string createMaterialForAssimpMat(const aiMaterial* aiMat, const MaterialLayout& matLayout);
    std::string createFallbackMaterialName();
    void centerAllMeshes() {};
    void generateLODs(std::vector<uint32_t>& indices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const;
    void loadMaterialTextures(const aiMaterial* aiMat,
        std::shared_ptr<Material> material,
        const MaterialLayout& matLayout,
        const std::string& directory);
    void processAssimpMesh(
        const aiScene* scene,
        const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        int meshIndex,
        const std::string& directory);

    // Helper function to convert aiMatrix4x4 to glm::mat4
    glm::mat4 AiToGlm(const aiMatrix4x4& m);
};