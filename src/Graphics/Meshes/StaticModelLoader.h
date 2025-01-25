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

//probably materials should be created like unordered_map[model_name] = std::vector<int> materials; for proper access, deletion
/**
 * @brief Container for a single mesh plus the associated material name.
 */


namespace staticloader
{
    class ModelLoader
    {
    public:
        ModelLoader(float scaleFactor = 1.0, std::unordered_map<aiTextureType, TextureType> aiToMyType = {
                { aiTextureType_DIFFUSE,  TextureType::Albedo      },
                { aiTextureType_NORMALS,  TextureType::Normal      },
                { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
                { aiTextureType_EMISSIVE, TextureType::Emissive    },
                { aiTextureType_AMBIENT, TextureType::Emissive    },
            });
        ~ModelLoader();

        //probably should add something for lod loader

        bool LoadStaticModel(const std::string& modelNam,
            const MeshLayout& meshLayout,
            const MaterialLayout& matLayout,
            bool centerModel);

        const std::vector<MeshInfo>& GetLoadedObjects() const { return m_Objects; }
        const std::vector<std::shared_ptr<Material>>& GetLoadedMaterials() const { return m_Materials; }

    private:
        std::vector<MeshInfo> m_Objects;
        std::vector<std::shared_ptr<Material>> m_Materials;

        int m_FallbackMaterialCounter = 0;
        int m_UnnamedMaterialCounter = 0;
        float m_ScaleFactor = 1.0;

        //sometimes texture labels are off, this is custom matching
        std::unordered_map<aiTextureType, TextureType> m_AiToMyType;
        
        const std::unordered_map<std::string, std::string> m_ModelPaths = {
            {"pig",    "../assets/Objs/pig_triangulated.obj"},
            {"bunny",  "../assets/Objs/bunny.obj"},
            {"dragon", "../assets/Objs/dragon.obj"},
            {"bistro", "../assets/AmazonBistro/Exterior/exterior.obj"},
            {"helmet", "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
        };

    private:
        std::string GetModelPath(const std::string& modelName);
        glm::mat4 AiToGlm(const aiMatrix4x4& m);

        //think about const
        void LoadSceneMaterials(const aiScene* scene,
            const MaterialLayout& matLayout,
            const std::string& directory);
        std::shared_ptr<Material> CreateMaterialForAssimpMat(const aiMaterial* aiMat,
            const MaterialLayout& matLayout,
            const std::string& directory);
        void LoadMaterialProperties(const aiMaterial* aiMat,
            std::shared_ptr<Material> mat,
            const MaterialLayout& matLayout);
        void LoadMaterialTextures(const aiMaterial* aiMat,
            std::shared_ptr<Material> mat,
            const MaterialLayout& matLayout,
            const std::string& directory);
        std::shared_ptr<Mesh> ProcessAssimpMesh(const aiMesh* aimesh,
            const MeshLayout& meshLayout,
            const glm::mat4& transform);
        void GenerateLODs(const std::vector<uint32_t>& srcIndices,
            const std::vector<float>& vertices3f,
            std::vector<std::vector<uint32_t>>& outLods) const;
    };
}
