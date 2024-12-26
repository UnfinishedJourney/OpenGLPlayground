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
#include "Graphics/Meshes/BetterModelLoader.h"
#include "Resources/MaterialManager.h"
#include "Graphics/Materials/MaterialLayout.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Clear();

    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Lights
    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    void UpdateLightsData();
    void BindLightSSBO() const;

    // Post-Processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    // Loading
    bool LoadModelIntoScene(const std::string& modelName,
        const std::string& defaultShaderName,
        const std::string& defaultMaterialName);

    // Batching
    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    // Frame data
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    // Culling + LOD
    void CullAndLODUpdate();

    // Access
    const std::vector<std::string>& GetMaterials() const { return m_LoadedMaterials; }
    void SetBGrid(bool bGrid) { m_BGrid = bGrid; }
    bool GetBGrid() const { return m_BGrid; }

    void SetBDebugLights(bool bDebugLights) { m_BDebugLights = bDebugLights; }
    bool GetBDebugLights() const { return m_BDebugLights; }

private:
    SceneGraph m_SceneGraph;
    bool m_StaticBatchesDirty = true;
    BatchManager m_StaticBatchManager;
    MeshLayout m_MeshLayout;
    std::shared_ptr<Camera> m_Camera;

    std::vector<LightData> m_LightsData;
    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    PostProcessingEffectType m_PostProcessingEffect = PostProcessingEffectType::None;

    std::shared_ptr<LODEvaluator> m_LODEvaluator;
    std::shared_ptr<FrustumCuller> m_FrustumCuller;

    bool m_BGrid = false;
    bool m_BDebugLights = false;

    std::vector<MeshInfo> m_LoadedMeshes;
    std::vector<std::string> m_LoadedMaterials;
    std::string m_LastShader;

    // Mapping: node index -> vector of newly created RenderObjects
    std::vector<std::vector<std::shared_ptr<RenderObject>>> m_NodeToRenderObjects;
};