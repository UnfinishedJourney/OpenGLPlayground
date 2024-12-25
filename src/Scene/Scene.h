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
#include "Graphics/Meshes/BetterModelLoader.h"
#include "Graphics/Meshes/Model.h"
#include "Resources/MaterialManager.h"
#include "Graphics/Materials/MaterialLayout.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Clear();

    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    void SetBGrid(bool bGrid) { m_BGrid = bGrid; }
    bool GetBGrid() const { return m_BGrid; }

    void SetBDebugLights(bool bDebugLights) { m_BDebugLights = bDebugLights; }
    bool GetBDebugLights() const { return m_BDebugLights; }

    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }

    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    bool LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName);

    void UpdateLightsData();
    void BindLightSSBO() const;

    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    void CullAndLODUpdate();

    const std::vector<std::string>& GetMaterials() const
    {
        return m_LoadedMaterials;
    }

private:
    SceneGraph m_SceneGraph;
    bool m_StaticBatchesDirty = true;
    BatchManager m_StaticBatchManager;
    MeshLayout m_MeshLayout;
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
    std::vector<std::string> m_LoadedMaterials; 
    std::string m_LastShader;

    // Map of model names to file paths
    std::unordered_map<std::string, std::string> m_ModelPaths = {
        {"pig", "../assets/Objs/pig_triangulated.obj"},
        {"bunny","../assets/Objs/bunny.obj"},
        {"dragon","../assets/Objs/dragon.obj"},
        {"bistro","../assets/AmazonBistro/Exterior/exterior.obj"},
        {"helmet","../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
    };

};