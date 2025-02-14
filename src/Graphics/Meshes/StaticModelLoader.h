#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>      // for uint8_t
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "MeshInfo.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialParamType.h"
#include "Graphics/Textures/ITexture.h"

namespace StaticLoader {

    /**
     * @brief Loads a model from disk (via Assimp) as a set of Mesh + Material pairs,
     *        baking transforms, optionally centering geometry, and generating LODs.
     */
    class ModelLoader {
    public:
        /**
         * @param scaleFactor  Uniform scale applied to all vertices before baking the transform.
         * @param aiToMyType   Mapping from Assimp texture types to your engine’s texture types.
         * @param maxLODs      Maximum number of LOD levels to generate (defaults to 5).
         */
        ModelLoader(float scaleFactor = 1.0f,
            std::unordered_map<aiTextureType, TextureType> aiToMyType = {
                { aiTextureType_DIFFUSE,  TextureType::Albedo },
                { aiTextureType_NORMALS,  TextureType::Normal },
                { aiTextureType_SPECULAR, TextureType::MetalRoughness },
                { aiTextureType_EMISSIVE, TextureType::Emissive },
                { aiTextureType_AMBIENT,  TextureType::Ambient },
                { aiTextureType_HEIGHT,   TextureType::Height }
            },
            uint8_t maxLODs = 5);
        ~ModelLoader() = default;

        /**
         * @brief Loads the specified model (by name), producing sub-meshes with baked transforms.
         *
         * @param modelName   A key mapping to an actual file path (from an internal registry).
         * @param meshLayout  Which vertex attributes to load (positions, normals, etc.).
         * @param matLayout   Which material parameters and textures are relevant.
         * @param centerModel If true, the geometry is recentered to the bounding box’s midpoint.
         * @return true on success, false otherwise.
         */
        bool LoadStaticModel(const std::string& modelName,
            const MeshLayout& meshLayout,
            const MaterialLayout& matLayout,
            bool centerModel = false);

        /**
         * @brief Returns the loaded MeshInfo objects (each holding a Mesh pointer and a material ID).
         */
        const std::vector<graphics::MeshInfo>& GetLoadedObjects() const { return objects_; }

    private:
        // Configuration parameters.
        float scaleFactor_ = 1.0f;
        std::unordered_map<aiTextureType, TextureType> aiToMyType_;
        uint8_t maxLODs_ = 8;

        // Loaded objects and associated material IDs.
        std::vector<graphics::MeshInfo> objects_;
        std::vector<std::size_t> materialIDs_;

        // Counters for fallback and unnamed materials.
        int fallbackMaterialCounter_ = 0;
        int unnamedMaterialCounter_ = 0;

        // Mapping from model name to file path.
        const std::unordered_map<std::string, std::string> modelPaths_ = {
            {"pig",    "../assets/Objs/pig_triangulated.obj"},
            {"bunny",  "../assets/Objs/bunny.obj"},
            {"duck",   "../assets/rubber_duck/scene.gltf"},
            {"dragon", "../assets/Objs/dragon.obj"},
            {"bistro", "../assets/AmazonBistro/Exterior/exterior.obj"},
            {"helmet", "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
        };

        // Private helper functions.
        std::string GetModelPath(const std::string& modelName) const;
        glm::mat4 AiToGlm(const aiMatrix4x4& m) const;

        void LoadSceneMaterials(const aiScene* scene,
            const MaterialLayout& matLayout,
            const std::string& directory);
        std::size_t CreateMaterialForAssimpMat(const aiMaterial* aiMat,
            const MaterialLayout& matLayout,
            const std::string& directory);
        void LoadMaterialProperties(const aiMaterial* aiMat,
            const std::unique_ptr<graphics::Material>& mat,
            const MaterialLayout& matLayout);
        void LoadMaterialTextures(const aiMaterial* aiMat,
            const std::unique_ptr<graphics::Material>& mat,
            const MaterialLayout& matLayout,
            const std::string& directory);

        std::shared_ptr<graphics::Mesh> ProcessAssimpMesh(const aiMesh* aimesh,
            const MeshLayout& meshLayout,
            const glm::mat4& transform);

        void GenerateLODs(std::vector<uint32_t> srcIndices,
            const std::vector<float>& vertices3f,
            std::vector<std::vector<uint32_t>>& outLods) const;

        void CenterMeshes();  ///< Shifts all loaded meshes so that the bounding box is centered at the origin.
    };

} // namespace staticloader