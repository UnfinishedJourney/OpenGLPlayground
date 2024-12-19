#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "SceneGraph.h"
#include "Camera.h"
#include "Lights.h"
#include "Renderer/BatchManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"
#include "LODEvaluator.h"
#include "FrustumCuller.h"
#include "Graphics/Meshes/ModelLoader.h"
#include "Graphics/Meshes/Model.h"
#include "Resources/MaterialManager.h"
#include "Graphics/Materials/MaterialLayout.h"

class Scene {
public:
    Scene();
    ~Scene();

    // Camera Management
    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Model Loading
    bool LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName, const MeshLayout& meshLayout, const MaterialLayout& layout);

    // Light Management
    void AddLight(const LightData& light);
    void UpdateLightsData();
    void BindLightSSBO() const;

    // Batch Management
    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    // Scene Management
    void Clear();

    // Frame Data
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    // Culling and LOD
    void CullAndLODUpdate();

    // Post-Processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    // Light Data Access
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    bool GetBDebugLights() const { return m_BDebugLights; }
    bool GetBGrid() const { return m_BGrid; }
    void SetBGrid(bool bGrid) { m_BGrid = bGrid; }
    void SetBDebugLights(bool bDebugLights) { m_BDebugLights = bDebugLights; }

private:
    SceneGraph m_SceneGraph;
    bool m_StaticBatchesDirty = true;
    BatchManager m_StaticBatchManager;

    std::shared_ptr<Camera> m_Camera;
    std::vector<LightData> m_LightsData;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    bool m_BGrid = false;
    bool m_BDebugLights = false;
    PostProcessingEffectType m_PostProcessingEffect;

    std::shared_ptr<LODEvaluator> m_LODEvaluator;
    std::shared_ptr<FrustumCuller> m_FrustumCuller;

    std::vector<MeshInfo> m_LoadedMeshes;
    std::vector<std::string> m_LoadedMaterials; // Actual material names from model
    std::string m_DefaultShader;
    std::string m_DefaultMaterial;

    // Map of model names to file paths
    std::unordered_map<std::string, std::string> m_ModelPaths = {
        {"pig", "../assets/Objs/pig_triangulated.obj"},
        {"bunny","../assets/Objs/bunny.obj"},
        {"dragon","../assets/Objs/dragon.obj"},
        {"bistro","../assets/AmazonBistro/Exterior/exterior.obj"},
        {"helmet","../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
    };

    void buildBatchesFromSceneGraph();
};