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
#include <glm/gtc/matrix_transform.hpp> // For glm::translate

// Forward Declarations
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
#include "Graphics/Textures/ITexture.h" // Assuming ITexture is the interface for textures
#include "Scene/SceneGraph.h"
#include "Utilities/Logger.h"
#include "Resources/MaterialManager.h"
#include "Resources/TextureManager.h"
#include "Graphics/Textures/TextureData.h"
/**
 * A container for the final loaded data:
 *  - The meshes
 *  - A parallel array of material names
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
};

/**
 * Represents textures associated with a mesh, categorized by texture type.
 */
struct BetterMeshTextures {
    std::unordered_map<TextureType, std::shared_ptr<ITexture>> textures;
};

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


/**
 * The BetterModelLoader class is responsible for loading 3D models using Assimp,
 * processing their meshes, materials, and textures, and integrating them into the SceneGraph.
 */
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

    /**
     * Retrieves the loaded model data.
     * @return A reference to the BetterModelData struct containing meshes and materials.
     */
    const BetterModelData& GetModelData() const {
        return m_Data;
    }

    /**
     * Retrieves the file path for a given model name.
     * @param modelName The name identifier for the model.
     * @return The corresponding file path as a string.
     */
    static std::string GetModelPath(const std::string& modelName);

private:
    // Member Variables
    BetterModelData m_Data;
    int m_FallbackMaterialCounter;

    // Private Methods for Scene Graph Integration
    void processAssimpNode(
        const aiScene* scene,
        const aiNode* ainode,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        const std::string& directory,
        SceneGraph& sceneGraph,
        int parentNode = -1
    );

    // Private Methods for Loading
    void loadSceneMaterials(const aiScene* scene, const MaterialLayout& matLayout, const std::string& directory);
    BetterMeshTextures LoadMeshTextures(const aiMaterial* material, const std::string& directory);
    std::string createMaterialForAssimpMat(const aiMaterial* aiMat, const MaterialLayout& matLayout, const std::string& directory);
    std::string createFallbackMaterialName();
    std::shared_ptr<Material> createFallbackMaterial(const std::string& name, const MaterialLayout& matLayout);
    void loadMaterialProperties(const aiMaterial* aiMat, std::shared_ptr<Material> mat, const MaterialLayout& matLayout);
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
    void generateLODs(const std::vector<uint32_t>& srcIndices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const;
    void setNodeBoundingVolumes(const aiScene* scene, const aiNode* ainode, int currentSGNode, SceneGraph& sceneGraph);
    void centerScene(SceneGraph& sceneGraph);

    // Helper Function
    glm::mat4 AiToGlm(const aiMatrix4x4& m);

    /**
     * Ensures that each material name is unique by appending a counter if necessary.
     * @param baseName The original material name.
     * @return A unique material name.
     */
    std::string ensureUniqueMaterialName(const std::string& baseName);
};