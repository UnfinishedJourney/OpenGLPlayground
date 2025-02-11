#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "MeshInfo.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialParamType.h"
#include "Graphics/Textures/ITexture.h"

namespace staticloader
{
    /**
     * @brief Loads a model from disk (via Assimp) as a set of Mesh + Material pairs,
     *        bakes transforms, optionally centers the geometry, and generates LODs.
     */
    class ModelLoader
    {
    public:
        /**
         * @param scaleFactor  Uniform scale applied to all vertices before baking transform.
         * @param aiToMyType   Mapping from Assimp texture type -> your engine’s texture type.
         * @param maxLODs      Maximum number of LOD levels to generate (defaults to 8).
         */
        ModelLoader(float scaleFactor = 1.0f,
            std::unordered_map<aiTextureType, TextureType> aiToMyType = {
                { aiTextureType_DIFFUSE,   TextureType::Albedo       },
                { aiTextureType_NORMALS,   TextureType::Normal       },
                { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
                { aiTextureType_EMISSIVE,  TextureType::Emissive     },
                { aiTextureType_AMBIENT,   TextureType::Emissive     }
            },
            uint8_t maxLODs = 5);

        ~ModelLoader() = default;

        /**
         * @brief Loads the specified model (by name), producing sub-meshes with transforms baked.
         * @param modelName     A key that maps to an actual file path in the internal registry.
         * @param meshLayout    Which attributes you want in the loaded mesh (positions, normals, etc.).
         * @param matLayout     Which material params or textures are relevant.
         * @param centerModel   If true, the geometry is recentered to the bounding box’s midpoint.
         * @return True on success, false otherwise.
         */
        bool LoadStaticModel(const std::string& modelName,
            const MeshLayout& meshLayout,
            const MaterialLayout& matLayout,
            bool centerModel = false);

        /**
         * @return Vector of MeshInfo objects (Mesh pointer + material index).
         */
        const std::vector<MeshInfo>& GetLoadedObjects() const { return m_Objects; }

        /**
         * @return The loaded materials for each sub-mesh in the same order
         *         they were discovered in the scene.
         */

    private:
        // Internal data
        float m_ScaleFactor = 1.0f;
        std::unordered_map<aiTextureType, TextureType> m_AiToMyType;
        uint8_t m_MaxLODs = 8;

        std::vector<MeshInfo>                m_Objects;
        std::vector<int> m_MaterialIDs;

        int m_FallbackMaterialCounter = 0;
        int m_UnnamedMaterialCounter = 0;

        // Map from model name to actual file path
        const std::unordered_map<std::string, std::string> m_ModelPaths = {
            {"pig",    "../assets/Objs/pig_triangulated.obj"},
            {"bunny",  "../assets/Objs/bunny.obj"},
            {"duck",  "../assets/rubber_duck/scene.gltf"},
            {"dragon", "../assets/Objs/dragon.obj"},
            {"bistro", "../assets/AmazonBistro/Exterior/exterior.obj"},
            {"helmet", "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
        };

    private:
        // Internal helpers
        std::string GetModelPath(const std::string& modelName) const;
        glm::mat4   AiToGlm(const aiMatrix4x4& m) const;

        void        LoadSceneMaterials(const aiScene* scene,
            const MaterialLayout& matLayout,
            const std::string& directory);
        int CreateMaterialForAssimpMat(const aiMaterial* aiMat,
            const MaterialLayout& matLayout,
            const std::string& directory);
        void        LoadMaterialProperties(const aiMaterial* aiMat,
            const std::unique_ptr<Graphics::Material>& mat,
            const MaterialLayout& matLayout);
        void        LoadMaterialTextures(const aiMaterial* aiMat,
            const std::unique_ptr<Graphics::Material>& mat,
            const MaterialLayout& matLayout,
            const std::string& directory);

        std::shared_ptr<Mesh> ProcessAssimpMesh(const aiMesh* aimesh,
            const MeshLayout& meshLayout,
            const glm::mat4& transform);

        void GenerateLODs(std::vector<uint32_t> srcIndices,
            const std::vector<float>& vertices3f,
            std::vector<std::vector<uint32_t>>& outLods) const;

        void CenterMeshes(); ///< Shifts all loaded meshes so bounding box is centered at origin
    };
} // namespace staticloader