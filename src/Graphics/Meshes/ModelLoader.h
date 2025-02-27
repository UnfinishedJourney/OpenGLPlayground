﻿//#pragma once
//
//#include <string>
//#include <vector>
//#include <memory>
//#include <unordered_map>
//#include <unordered_set>
//#include <glm/glm.hpp>
//#include <assimp/scene.h>       
//#include "Graphics/Meshes/Mesh.h"
//#include "Graphics/Meshes/MeshLayout.h"
//#include "Graphics/Materials/MaterialLayout.h"
//#include "Graphics/Materials/Material.h"
//#include "Graphics/Materials/MaterialParamType.h"
//#include "Graphics/Textures/ITexture.h"
//#include "Graphics/Textures/TextureData.h"
//#include "Scene/SceneGraph.h"
//#include "Utilities/Logger.h"
//#include "Graphics/Materials/MaterialManager.h"
//#include "Graphics/Textures/TextureManager.h"
//
//#include "MeshInfo.h"
//
///**
// * @brief Container for a single mesh plus the associated material name.
// */
//struct BetterModelMeshData
//{
//    std::shared_ptr<Mesh> mesh;
//    std::string           materialName;
//};
//
///**
// * @brief Holds all final data loaded by ModelLoader: the meshes and any created materials.
// */
//struct BetterModelData
//{
//    std::vector<BetterModelMeshData> meshesData;
//    std::vector<std::string>         createdMaterials;
//};
//
///**
// * @brief Holds textures loaded for a single mesh, categorized by TextureType.
// */
//struct BetterMeshTextures
//{
//    std::unordered_map<TextureType, std::shared_ptr<graphics::ITexture>> textures;
//};
//
///*
//* We load model based on mesh layout and matlayout we get from the shader
//* we either move center to 0 or not
//* load everything to scene graph which i might not need for now
//*/
//
////maybe materials should also have some sort of int id additionally to string
//class ModelLoader
//{
//public:
//    ModelLoader(float scaleFactor = 1.0, std::unordered_map<aiTextureType, TextureType> aiToMyType = {
//            { aiTextureType_DIFFUSE,  TextureType::Albedo      },
//            { aiTextureType_NORMALS,  TextureType::Normal      },
//            { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
//            { aiTextureType_EMISSIVE, TextureType::Emissive    },
//            { aiTextureType_AMBIENT, TextureType::Emissive    },
//        });
//    ~ModelLoader();
//
//    //probably should add something for lod loader
//    bool LoadModel(const std::string& filePath,
//        const MeshLayout& meshLayout,
//        const MaterialLayout& matLayout,
//        SceneGraph& sceneGraph,
//        bool centerModel);
//
//    const BetterModelData& GetModelData() const { return m_Data; }
//
//    static std::string GetModelPath(const std::string& modelName);
//
//private:
//    //maybe should use unique_ptr or something like that
//    BetterModelData m_Data;
//    int m_FallbackMaterialCounter = 0;
//    float m_ScaleFactor = 1.0;
//private:
//    void LoadSceneMaterials(const aiScene* scene,
//        const MaterialLayout& matLayout,
//        const std::string& directory);
//
//    std::string CreateMaterialForAssimpMat(const aiMaterial* aiMat,
//        const MaterialLayout& matLayout,
//        const std::string& directory);
//
//    std::string EnsureUniqueMaterialName(const std::string& baseName);
//    void LoadMaterialProperties(const aiMaterial* aiMat,
//        std::shared_ptr<graphics::Material> mat,
//        const MaterialLayout& matLayout);
//    void LoadMaterialTextures(const aiMaterial* aiMat,
//        std::shared_ptr<graphics::Material> material,
//        const MaterialLayout& matLayout,
//        const std::string& directory);
//
//    /**
//     * @brief Actually loads the texture files from disk, returning a map of TextureType -> ITexture
//     */
//    BetterMeshTextures LoadMeshTextures(const aiMaterial* material,
//        const std::string& directory);
//
//    std::string CreateFallbackMaterialName();
//    std::shared_ptr<graphics::Material> CreateFallbackMaterial(const std::string& name,
//        const MaterialLayout& matLayout);
//
//    void ProcessAssimpNode(const aiScene* scene,
//        const aiNode* ainode,
//        const MeshLayout& meshLayout,
//        const MaterialLayout& matLayout,
//        const std::string& directory,
//        SceneGraph& sceneGraph,
//        int parentNode = -1);
//
//    void ProcessAssimpMesh(const aiScene* scene,
//        const aiMesh* aimesh,
//        const MeshLayout& meshLayout,
//        int meshIndex,
//        const std::string& directory);
//
//    void GenerateLODs(const std::vector<uint32_t>& srcIndices,
//        const std::vector<float>& vertices3f,
//        std::vector<std::vector<uint32_t>>& outLods) const;
//
//    void SetNodeBoundingVolumes(const aiScene* scene,
//        const aiNode* ainode,
//        int currentSGNode,
//        SceneGraph& sceneGraph);
//
//    void CenterScene(SceneGraph& sceneGraph);
//
//    glm::mat4 AiToGlm(const aiMatrix4x4& m);
//
//    std::unordered_map<aiTextureType, TextureType> m_AiToMyType;
//};