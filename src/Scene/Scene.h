#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

// Forward declarations or includes
#include "Scene/SceneGraph.h"
#include "Scene/Transform.h"
#include "Scene/Camera.h"
#include "Scene/Lights.h"
#include "Renderer/RenderObject.h"
#include "Renderer/BatchManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/TextureManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Textures/TextureData.h"
#include "Graphics/Textures/ITexture.h"
#include "FrustumCuller.h"
#include "LODEvaluator.h"

// Old, “dynamic” (hierarchical) loader:
#include "Graphics/Meshes/ModelLoader.h"

// New, “static” loader (bakes transforms):
#include "Graphics/Meshes/StaticModelLoader.h"

/**
 * @brief Represents the entire scene: possibly with a SceneGraph (for dynamic/hierarchical objects),
 *        plus a list of “static” objects whose transforms are baked in.
 */
class Scene
{
public:
    Scene();
    ~Scene();

    void Clear();

    // Camera handling
    void SetCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Lights
    void AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return m_LightsData; }
    void BindLightSSBO() const;

    // Post-processing
    void SetPostProcessingEffect(PostProcessingEffectType effect);
    PostProcessingEffectType GetPostProcessingEffect() const;

    /**
     * @brief Loads a model using the **old** hierarchical ModelLoader (with a SceneGraph).
     *        This is for “dynamic”/skinned or otherwise transform-hierarchical objects.
     * @param modelName  A key that resolves to a path in ModelLoader::GetModelPath
     * @param defaultShaderName The shader to use for the loaded meshes
     * @param defaultMaterialName You can override, or let the loader create materials
     * @param scaleFactor   Scales the model
     * @param aiToMyType    Map from Assimp texture types to your engine’s texture types
     */
    //bool LoadModelIntoScene(const std::string& modelName,
    //    const std::string& defaultShaderName,
    //    const std::string& defaultMaterialName,
    //    float scaleFactor = 1.0f,
    //    std::unordered_map<aiTextureType, TextureType> aiToMyType =
    //    {
    //        { aiTextureType_DIFFUSE,   TextureType::Albedo       },
    //        { aiTextureType_NORMALS,   TextureType::Normal       },
    //        { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
    //        { aiTextureType_EMISSIVE,  TextureType::Emissive     },
    //        { aiTextureType_AMBIENT,   TextureType::Emissive     },
    //    });

    /**
     * @brief Loads a model using the **new** staticloader::ModelLoader (no SceneGraph).
     *        The entire hierarchy is baked into the final vertex transforms.
     * @param modelName   A key that resolves to a path in the static loader’s registry
     * @param shaderName  The shader to use
     * @param scaleFactor Scales the final geometry
     * @param aiToMyType  Mapping from Assimp texture type -> engine’s texture type
     */
    bool LoadStaticModelIntoScene(const std::string& modelName,
        const std::string& shaderName,
        float scaleFactor = 1.0f,
        std::unordered_map<aiTextureType, TextureType> aiToMyType =
        {
            { aiTextureType_DIFFUSE,   TextureType::Albedo       },
            { aiTextureType_NORMALS,   TextureType::Normal       },
            { aiTextureType_SPECULAR,  TextureType::MetalRoughness },
            { aiTextureType_EMISSIVE,  TextureType::Emissive     },
            { aiTextureType_AMBIENT,   TextureType::Emissive     },
        });

    // Batching
    void BuildStaticBatchesIfNeeded();
    const std::vector<std::shared_ptr<Batch>>& GetStaticBatches() const;

    // Per-frame uniform updates
    void UpdateFrameDataUBO() const;
    void BindFrameDataUBO() const;

    // Culling & LOD
    void CullAndLODUpdate();

    // Accessors
    const std::vector<int>& GetMaterials() const { return m_LoadedMaterials; }

    void SetBGrid(bool bGrid) { m_BGrid = bGrid; }
    bool GetBGrid() const { return m_BGrid; }
    void SetBDebugLights(bool bDebugLights) { m_BDebugLights = bDebugLights; }
    bool GetBDebugLights() const { return m_BDebugLights; }

    bool GetBSkybox() const { return m_EnableSkybox; }
    void SetBSkybox(bool bSkyBox) { m_EnableSkybox = bSkyBox; }

private:
    // Hierarchical data (if you still want it):
    SceneGraph m_SceneGraph;

    /**
     * “Static” objects (transforms baked in). We store them simply
     * as a list of RenderObjects with identity transforms or single transforms.
     */
    std::vector<std::shared_ptr<BaseRenderObject>> m_StaticObjects;

    /**
     * The batch manager for static geometry. If you want to batch
     * them by material, etc., you can do so here.
     */
    BatchManager m_StaticBatchManager;
    bool         m_StaticBatchesDirty = true;

    // Global toggles
    bool m_EnableSkybox = false;
    bool m_BGrid = false;
    bool m_BDebugLights = false;

    // Materials and Meshes loaded (both dynamic and static)
    std::vector<int> m_LoadedMaterials; // For reference
    std::vector<MeshInfo>    m_LoadedMeshes;    // For the old approach (SceneGraph indexing)

    // For dynamic->node referencing
    std::vector<std::vector<std::shared_ptr<RenderObject>>> m_NodeToRenderObjects;

    // Last used shader name in the load call
    std::string m_LastShader;
    MeshLayout m_LastLayout;
    // Camera, lights, effect
    std::shared_ptr<Camera> m_Camera;
    std::vector<LightData>  m_LightsData;
    PostProcessingEffectType m_PostProcessingEffect = PostProcessingEffectType::None;

    // Buffers for frame data and lights
    std::unique_ptr<UniformBuffer>       m_FrameDataUBO;
    std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;

    // LOD & culling
    std::shared_ptr<LODEvaluator>  m_LODEvaluator;
    std::shared_ptr<FrustumCuller> m_FrustumCuller;

private:
    // Helpers
    void UpdateLightsData();
};