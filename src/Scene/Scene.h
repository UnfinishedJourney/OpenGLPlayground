#pragma once

#include <vector>
#include <memory>
#include "Renderer/RenderObject.h"
#include "Renderer/BatchManager.h"
#include "Scene/Lights.h"
#include "Scene/Camera.h"
#include "Scene/LODEvaluator.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"
#include "Graphics/Textures/OpenGLTexture.h"

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

class Scene {
public:
    Scene();
    ~Scene();

    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void AddLight(const LightData& light);
    void SetCamera(const std::shared_ptr<Camera>& camera);
    void Clear();

    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;
    void UpdateFrameDataUBO() const;
    void UpdateLightsData();
    void BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const;

    void BuildBatches() const;
    void BindLightSSBO() const;
    void BindFrameDataUBO() const;
    void SetLOD(size_t newLOD)
    {
        m_BatchManager.SetLOD(newLOD);
    }

    void UpdateLODs()
    {
        m_BatchManager.UpdateLODs(m_Camera, m_LODEvaluator);
    }

    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    void SetBDebugLights(bool bDebugLights)
    {
        m_BDebugLights = bDebugLights;
    }

    bool GetBDebugLights() const
    {
        return m_BDebugLights;
    }

    void SetBGrid(bool bGrid)
    {
        m_BGrid = bGrid;
    }

    bool GetBGrid() const
    {
        return m_BGrid;
    }

    std::vector<LightData> GetLights() const
    {
        return m_LightsData;
    }

    std::shared_ptr<Camera> GetCamera() { return m_Camera; }

    void SetTerrainHeightMap(const std::shared_ptr<ITexture>& heightMap);
    std::shared_ptr<ITexture> GetTerrainHeightMap() const;

private:

    bool m_BDebugLights = false;
    bool m_BGrid = false;
    const int MAX_LIGHTS = 8;
    std::vector<LightData> m_LightsData;
    std::shared_ptr<Camera> m_Camera;
    LODEvaluator m_LODEvaluator;
    mutable BatchManager m_BatchManager;
    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;
    PostProcessingEffectType m_PostProcessingEffect;
    std::shared_ptr<ITexture> m_TerrainHeightMap = nullptr;
};