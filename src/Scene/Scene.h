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

static const size_t MAX_LIGHTS = 32;

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

class Scene {
public:
    Scene();
    ~Scene();

    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Root node of the scene graph
    std::shared_ptr<SceneNode> GetRootNode() const { return m_RootNode; }

    // Lights
    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    void UpdateLightsData();
    void BindLightSSBO() const;

    // Build the final batch (once all nodes / culling / LOD is decided)
    void BuildBatches();
    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;

    // Clear scene
    void Clear();

    // UBO / SSBO
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    // LOD pass
    void UpdateLODs()
    {
        m_BatchManager.UpdateLODs(m_Camera, m_LODEvaluator);
    }

    // Post-processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    // Debug toggles
    void SetBGrid(bool b) { m_BGrid = b; }
    bool GetBGrid() const { return m_BGrid; }
    void SetBDebugLights(bool b) { m_BDebugLights = b; }
    bool GetBDebugLights() const { return m_BDebugLights; }
    std::vector<LightData> GetLights() const { return m_LightsData; }
    // Terrain heightmap
    void SetTerrainHeightMap(const std::shared_ptr<ITexture>& heightMap);
    std::shared_ptr<ITexture> GetTerrainHeightMap() const;

    // Scene-wide culling & LOD traversal if needed
    void TraverseSceneGraph(std::shared_ptr<SceneNode> node, const glm::mat4& parentTransform);

private:
    bool m_bFirstTraverse = true;
    std::shared_ptr<SceneNode> m_RootNode;
    std::shared_ptr<Camera> m_Camera;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    std::vector<LightData> m_LightsData;
    BatchManager m_BatchManager;

    bool m_BGrid = false;
    bool m_BDebugLights = false;
    PostProcessingEffectType m_PostProcessingEffect;

    std::shared_ptr<ITexture> m_TerrainHeightMap;
    LODEvaluator m_LODEvaluator;
};