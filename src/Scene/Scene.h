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
#include "Graphics/Meshes/ModelLoader.h"  // so we can reference BetterModelData if needed
#include "Graphics/Meshes/MeshInfo.h"  // ensure "MeshInfo" is visible

/**
 * @brief Represents the entire scene: SceneGraph, camera, lights, etc.
 */
class Scene
{
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
        const std::string& defaultMaterialName,
        float scaleFactor = 1.0,
        std::unordered_map<aiTextureType, TextureType> aiToMyType = {
            { aiTextureType_DIFFUSE,  TextureType::Albedo      },
            { aiTextureType_NORMALS,  TextureType::Normal      },
            { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
            { aiTextureType_EMISSIVE, TextureType::Emissive    },
            { aiTextureType_AMBIENT, TextureType::Emissive    },
        });

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

    bool GetBSkybox() const { return m_EnableSkybox; }
    bool SetBSkybox(bool bSkyBox) { m_EnableSkybox = bSkyBox; }

private:
    // SceneGraph holds hierarchical transforms, bounding volumes, etc.
    SceneGraph         m_SceneGraph;
    bool               m_StaticBatchesDirty = true;
    bool               m_EnableSkybox = false;
    BatchManager       m_StaticBatchManager;
    MeshLayout         m_MeshLayout;
    std::shared_ptr<Camera> m_Camera;

    // Lights
    std::vector<LightData>   m_LightsData;
    std::unique_ptr<UniformBuffer>       m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    PostProcessingEffectType m_PostProcessingEffect = PostProcessingEffectType::None;

    // LOD & culling
    std::shared_ptr<LODEvaluator>  m_LODEvaluator;
    std::shared_ptr<FrustumCuller> m_FrustumCuller;

    bool m_BGrid = false;
    bool m_BDebugLights = false;

    /**
     * List of loaded mesh references, each storing a pointer to the actual Mesh
     * and possibly a material index or other data.
     */
    std::vector<MeshInfo> m_LoadedMeshes;   // <--- now recognized

    std::vector<std::string> m_LoadedMaterials;
    std::string              m_LastShader;

    /**
     * Each node in SceneGraph can have multiple meshes =>
     * we store a vector of RenderObjects for each node index.
     */
    std::vector<std::vector<std::shared_ptr<RenderObject>>> m_NodeToRenderObjects;
};