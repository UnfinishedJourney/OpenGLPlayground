#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <assimp/scene.h>
#include "Graphics/Materials/MaterialParamType.h"

// Forward declarations for faster builds
class Camera;
class BaseRenderObject;
class RenderObject;
class StaticRenderObject;
class UniformBuffer;
class ShaderStorageBuffer;
class Batch;
class BatchManager;
class FrustumCuller;
class LODEvaluator;
class SceneGraph;

struct alignas(16) LightData {
    glm::vec4 position; // w is padding or another parameter, for dir lights position is direction? don't have them yet
    glm::vec4 color;    // w is intensity
};

// Enums or small structs that are used
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"

/**
 * @brief Represents the entire scene:
 *  - Possibly a SceneGraph (for dynamic/hierarchical objects),
 *  - A list of static objects (transforms baked in),
 *  - A camera & lights,
 *  - Post-processing info,
 *  - Batching logic.
 */
class Scene
{
public:
    Scene();
    ~Scene();

    // Clears all data (camera, lights, objects, etc.)
    void Clear();

    // Camera
    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Lights
    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    void BindLightSSBO() const; // Binds the SSBO with light data

    // Post-processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    /**
     * @brief Loads a model using a static approach: transforms are baked in the final vertices.
     * @param modelName   Key to a path in the static loader’s internal registry.
     * @param shaderName  The shader to use for rendering the loaded sub-meshes.
     * @param scaleFactor Uniform scaling factor applied before baking.
     * @return True on success, false otherwise.
     */
    bool LoadStaticModelIntoScene(const std::string& modelName,
        const std::string& shaderName,
        float scaleFactor = 1.0f,
        std::unordered_map<aiTextureType, TextureType> aiToMyType = {
                { aiTextureType_DIFFUSE,   TextureType::Albedo       },
                { aiTextureType_NORMALS,   TextureType::Normal       },
                { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
                { aiTextureType_EMISSIVE,  TextureType::Emissive     },
                { aiTextureType_AMBIENT,   TextureType::Emissive     }
        });

    // Batching
    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    // Per-frame uniform updates (like camera view/proj)
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO()  const;

    // LOD & Culling
    void CullAndLODUpdate();

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
    // Hierarchical approach (optional):
    std::unique_ptr<SceneGraph> m_SceneGraph; // or store by value if you prefer

    /**
     * @brief A list of "static" objects whose transforms are already baked.
     */
    std::vector<std::shared_ptr<BaseRenderObject>> m_StaticObjects;

    /**
     * @brief The manager that batches static geometry by material/shader/etc.
     */
    std::unique_ptr<BatchManager> m_StaticBatchManager;
    bool m_StaticBatchesDirty = true;

    // Camera & Lights
    std::shared_ptr<Camera> m_Camera;
    std::vector<LightData>  m_LightsData;

    // Post-processing effect
    PostProcessingEffectType m_PostProcessingEffect = PostProcessingEffectType::None;

    // Uniforms & SSBO
    std::unique_ptr<UniformBuffer>       m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    // LOD & culling
    std::unique_ptr<LODEvaluator>    m_LODEvaluator;
    std::unique_ptr<FrustumCuller>   m_FrustumCuller;

    // Some toggles
    bool m_EnableSkybox = false;
    bool m_ShowGrid = false;
    bool m_ShowDebugLights = false;
    bool m_TurnOnShadows = false;

    // Temp store last loaded shader name, etc., if needed
    std::string m_LastShaderName;

private:
    // Internal
    void UpdateLightsData();
};