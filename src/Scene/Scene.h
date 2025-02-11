#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "Renderer/Batch.h"
#include "Renderer/BatchManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Scene/Camera.h"
#include "Scene/FrustumCuller.h"
#include "Scene/LODEvaluator.h"
#include "Scene/SceneGraph.h"
#include "LightManager.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"

namespace Scene {
    /**
     * @brief Represents the entire scene:
     *  - Possibly a SceneGraph (for dynamic/hierarchical objects),
     *  - A list of static objects (transforms baked in),
     *  - A camera & a LightManager,
     *  - Batching logic,
     *  - Post-processing toggles.
     */
    class Scene
    {
    public:
        Scene();
        ~Scene();

        void Clear();

        // Camera
        void SetCamera(const std::shared_ptr<Camera>& camera);
        std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

        // Load a model or primitive
        bool LoadStaticModelIntoScene(
            const std::string& modelName,
            const std::string& shaderName,
            float scaleFactor = 1.0f,
            std::unordered_map<aiTextureType, TextureType> aiToMyType = {
                    { aiTextureType_DIFFUSE,   TextureType::Albedo       },
                    { aiTextureType_NORMALS,   TextureType::Normal       },
                    { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
                    { aiTextureType_EMISSIVE,  TextureType::Emissive     },
                    { aiTextureType_AMBIENT,   TextureType::Emissive     }
            });

        bool LoadPrimitiveIntoScene(
            const std::string& primitiveName,
            const std::string& shaderName,
            int materialID = 0
        );

        void BuildStaticBatchesIfNeeded();
        const std::vector<std::shared_ptr<Graphics::Batch>>& GetStaticBatches() const;

        // Per-frame uniform updates
        void UpdateFrameDataUBO() const;
        void BindFrameDataUBO()  const;

        // LOD & Culling
        void CullAndLODUpdate();

        // Light manager
        std::shared_ptr<LightManager> GetLightManager() const { return m_LightManager; }

        // Post-processing toggles
        void  SetPostProcessingEffect(PostProcessingEffectType effect);
        PostProcessingEffectType GetPostProcessingEffect() const;

        BoundingBox ComputeWorldBoundingBox() const;

        // Scene toggles
        void SetShowGrid(bool bGrid) { m_ShowGrid = bGrid; }
        bool GetShowGrid()             const { return m_ShowGrid; }

        void SetShowDebugLights(bool bDebug) { m_ShowDebugLights = bDebug; }
        bool GetShowDebugLights()      const { return m_ShowDebugLights; }

        void SetSkyboxEnabled(bool enable) { m_EnableSkybox = enable; }
        bool GetSkyboxEnabled()        const { return m_EnableSkybox; }

        void SetShowShadows(bool bShadows) { m_TurnOnShadows = bShadows; }
        bool GetShowShadows()             const { return m_TurnOnShadows; }

    private:
        // For hierarchical scenes (optional)
        std::unique_ptr<SceneGraph> m_SceneGraph;

        // Light manager
        std::shared_ptr<LightManager> m_LightManager;

        // A list of static objects
        std::vector<std::shared_ptr<BaseRenderObject>> m_StaticObjects;
        bool m_StaticBatchesDirty = true;

        // Batching
        std::unique_ptr<BatchManager> m_StaticBatchManager;

        // Camera
        std::shared_ptr<Camera> m_Camera;

        // Uniform buffer for frame data
        std::unique_ptr<Graphics::UniformBuffer> m_FrameDataUBO;

        // LOD & culling
        std::unique_ptr<LODEvaluator>   m_LODEvaluator;
        std::unique_ptr<FrustumCuller>  m_FrustumCuller;

        // Post-processing effect
        PostProcessingEffectType m_PostProcessingEffect = PostProcessingEffectType::None;

        // Possibly store the last used shader name, etc.
        std::string m_LastShaderName;
        // Some toggles
        bool m_EnableSkybox = false;
        bool m_ShowGrid = false;
        bool m_ShowDebugLights = false;
        bool m_TurnOnShadows = false;

    };
}