#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "SceneNode.h"
#include "Scene/Camera.h"
#include "Scene/Lights.h"
#include "Renderer/BatchManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"
#include "Scene/LODEvaluator.h"
//#include "Scene/FrustumCuller.h" // For culling

static const size_t MAX_LIGHTS = 32;

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

class FrustumCuller {
public:
    // For a more advanced approach, you'd extract planes from the camera's projection/view matrix.
    // For simplicity, let's define a helper function:
    bool IsSphereVisible(const glm::vec3& center, float radius, const std::shared_ptr<Camera>& camera) const {
        // Basic approach: check if (center) is within some distance from the camera
        // or do a real frustum-plane intersection test. We'll do a simple distance check:
        float distance = glm::distance(camera->GetPosition(), center);
        // If the distance - radius is beyond some huge range, we cull. E.g. 1000
        // (In real code, you'd extract planes from the camera frustum)
        float farPlane = camera->GetFarPlane();
        return (distance - radius) < farPlane;
    }
};

class Scene {
public:
    Scene();
    ~Scene();

    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Add static node => we build batches once
    void AddStaticNode(const std::shared_ptr<SceneNode>& node);

    // Potential dynamic approach
    void AddDynamicNode(const std::shared_ptr<SceneNode>& node);

    // Lights
    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    void UpdateLightsData();
    void BindLightSSBO() const;

    // Build static batches once
    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    // For dynamic objects (not implemented fully)
    // e.g. you might do separate draws or re-batch. Not shown for brevity.

    // Clear scene
    void Clear();

    // UBO / SSBO
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    // Per-frame: do culling + LOD
    void CullAndLODUpdate();

    // Post-processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    // Terrain
    void SetTerrainHeightMap(const std::shared_ptr<ITexture>& heightMap);
    std::shared_ptr<ITexture> GetTerrainHeightMap() const;

    // Debug toggles
    void SetBGrid(bool b) { m_BGrid = b; }
    bool GetBGrid() const { return m_BGrid; }

    void SetBDebugLights(bool b) { m_BDebugLights = b; }
    bool GetBDebugLights() const { return m_BDebugLights; }

    void SetLODEvaluator(const std::shared_ptr<LODEvaluator>& lodEval) { m_LODEvaluator = lodEval; }
    void SetFrustumCuller(const std::shared_ptr<FrustumCuller>& culler) { m_FrustumCuller = culler; }
    //const std::vector<LightData>& GetLightsData() const { return m_LightsData; }

private:
    std::vector<std::shared_ptr<SceneNode>> m_StaticNodes;
    std::vector<std::shared_ptr<SceneNode>> m_DynamicNodes; // optional for dynamic objects

    bool m_StaticBatchesDirty = true;
    BatchManager m_StaticBatchManager;

    std::shared_ptr<Camera> m_Camera;
    std::vector<LightData> m_LightsData;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    std::shared_ptr<ITexture> m_TerrainHeightMap;

    bool m_BGrid = false;
    bool m_BDebugLights = false;
    PostProcessingEffectType m_PostProcessingEffect;

    // LOD & Culling
    std::shared_ptr<LODEvaluator> m_LODEvaluator;
    std::shared_ptr<FrustumCuller> m_FrustumCuller;

    // Build logic
    void BuildStaticBatchNodes();
};