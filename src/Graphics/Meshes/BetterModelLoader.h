#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

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
    std::vector<BetterModelMeshData> meshesData;  ///< Each mesh + the associated material name
    std::vector<std::string>         createdMaterials; ///< All unique material names created
    // If you want node hierarchy, you can add it here (like in your older code).
};

/**
 * The BetterModelLoader class:
 *  1) Loads geometry using Assimp.
 *  2) Generates LOD levels using meshoptimizer.
 *  3) Creates fallback materials for any mesh that has no or invalid material.
 *  4) Binds textures if found (you can store them in the material).
 */
class BetterModelLoader
{
public:
    BetterModelLoader();
    ~BetterModelLoader();

    /**
     * Load a model from `filePath` using Assimp.
     *
     * \param filePath The path to the model file (OBJ, FBX, etc.).
     * \param meshLayout Which vertex attributes to load (positions, normals, etc.).
     * \param matLayout  Which material parameters (Ambient, Diffuse, etc.) and textures to load.
     * \param centerModel (Optional) Whether to recenter the mesh geometry around origin.
     */
    bool LoadModel(
        const std::string& filePath,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        bool centerModel = false);

    /**
     * Returns the final loaded data after calling LoadModel(...).
     */
    const BetterModelData& GetLoadedData() const { return m_Data; }

private:
    // A local container for final results
    BetterModelData m_Data;
    // Counters for fallback materials
    int             m_FallbackMaterialCounter;

private:
    // Helpers
    void loadSceneMaterials(const aiScene* scene, const MaterialLayout& matLayout);
    std::string createMaterialForAssimpMat(const aiMaterial* aiMat, const MaterialLayout& matLayout);

    void processAssimpMesh(const aiScene* scene, const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        int meshIndex,  // used for naming fallback material, if needed
        const std::string& directory);

    /**
     * Given raw index data and raw vertex data, produce multiple LOD levels using meshoptimizer.
     */
    void generateLODs(
        std::vector<uint32_t>& indices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const;

    /**
     * Load any textures from the aiMaterial -> store them in the newly created Material,
     * if the matLayout requires them.
     * If not found, do nothing or create fallback.
     */
    void loadMaterialTextures(
        const aiMaterial* aiMat,
        std::shared_ptr<Material> material,
        const MaterialLayout& matLayout,
        const std::string& directory);

    /**
     * If a mesh doesn’t have a valid aiMaterial index, create a fallback material
     * with a unique name: “FallbackMaterial_X”
     */
    std::string createFallbackMaterialName();

    /**
     * Optionally center the model if `centerModel = true`.
     * Implementation is up to you (shift all vertices by the bounding box’s center).
     */
    void centerAllMeshes();
};
