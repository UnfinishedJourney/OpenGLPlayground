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
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Scene/Camera.h"
#include "Scene/FrustumCuller.h"
#include "Scene/LODEvaluator.h"
#include "Scene/SceneGraph.h"
#include "LightManager.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"

// Forward declarations for types used below.
struct BoundingBox;            
class BaseRenderObject;        

namespace Scene {

    /**
     * @brief Represents the entire scene.
     *
     * This class encapsulates the scene graph for dynamic objects, a list of static objects,
     * the active camera, lighting, batching for efficient rendering, and post-processing settings.
     */
    class Scene
    {
    public:
        Scene();
        ~Scene();

        /// Clears the scene (removing all objects and resetting managers).
        void Clear();

        /// Sets the active camera.
        void SetCamera(const std::shared_ptr<Camera>& camera);
        /// Returns the active camera.
        std::shared_ptr<Camera> GetCamera() const { return camera_; }

        /**
         * @brief Loads a static model into the scene.
         *
         * @param modelName The name or path of the model to load.
         * @param shaderName The shader name used for rendering.
         * @param scaleFactor Scaling factor applied to the model.
         * @param aiToMyType Mapping from Assimp texture types to engine-specific texture types.
         * @return true if the model loaded successfully, false otherwise.
         */
        bool LoadStaticModelIntoScene(
            const std::string& modelName,
            const std::string& shaderName,
            float scaleFactor = 1.0f);

        /**
         * @brief Loads a primitive (predefined geometry) into the scene.
         *
         * @param primitiveName The name of the primitive.
         * @param shaderName The shader to be used.
         * @param materialID Material identifier.
         * @return true if the primitive was loaded successfully, false otherwise.
         */
        bool LoadPrimitiveIntoScene(
            const std::string& primitiveName,
            const std::string& shaderName,
            int materialID = 0
        );

        /// Builds static render batches if there have been changes.
        void BuildStaticBatchesIfNeeded();
        /// Returns the static render batches.
        const std::vector<std::shared_ptr<renderer::Batch>>& GetStaticBatches() const;

        /// Updates the per-frame UBO with current camera/view data.
        void UpdateFrameDataUBO() const;
        /// Binds the per-frame UBO.
        void BindFrameDataUBO() const;

        /// Performs frustum culling and updates Level-of-Detail (LOD).
        void CullAndLODUpdate();

        /// Returns the scene's light manager.
        std::shared_ptr<LightManager> GetLightManager() const { return lightManager_; }

        /// Sets the post-processing effect.
        void SetPostProcessingEffect(PostProcessingEffectType effect);
        /// Returns the current post-processing effect.
        PostProcessingEffectType GetPostProcessingEffect() const;

        /// Computes the world-space bounding box for static objects.
        BoundingBox ComputeWorldBoundingBox() const;

        // Scene toggle setters/getters.
        void SetShowGrid(bool show) { showGrid_ = show; }
        bool GetShowGrid() const { return showGrid_; }

        void SetShowDebugLights(bool show) { showDebugLights_ = show; }
        bool GetShowDebugLights() const { return showDebugLights_; }

        void SetSkyboxEnabled(bool enable) { enableSkybox_ = enable; }
        bool GetSkyboxEnabled() const { return enableSkybox_; }

        void SetShowShadows(bool show) { turnOnShadows_ = show; }
        bool GetShowShadows() const { return turnOnShadows_; }

        void SetShadowMapSize(int shadowSize) { shadowMapSize_ = shadowSize; }
        int GetShadowMapSize() const { return shadowMapSize_; }

    private:
        // Scene graph for dynamic/hierarchical objects.
        std::unique_ptr<SceneGraph> sceneGraph_;

        // Manager for lighting.
        std::shared_ptr<LightManager> lightManager_;

        // List of static render objects.
        std::vector<std::shared_ptr<BaseRenderObject>> staticObjects_;
        bool staticBatchesDirty_ = true;

        // Manager for batching static render objects.
        std::unique_ptr<BatchManager> staticBatchManager_;

        // The active camera.
        std::shared_ptr<Camera> camera_;

        // UBO for per-frame data (e.g. view/projection matrices).
        std::unique_ptr<graphics::UniformBuffer> frameDataUBO_;

        // SSBO for per-object data (e.g. model matrices).
        std::unique_ptr<graphics::ShaderStorageBuffer> objectDataSSBO_;

        // Evaluator for Level-of-Detail.
        std::unique_ptr<LODEvaluator> lodEvaluator_;
        // Frustum culler for visibility determination.
        std::unique_ptr<FrustumCuller> frustumCuller_;

        // Active post-processing effect.
        PostProcessingEffectType postProcessingEffect_ = PostProcessingEffectType::None;

        // Cache for last used shader name.
        std::string lastShaderName_;

        // Scene toggle flags.
        bool enableSkybox_ = false;
        bool showGrid_ = false;
        bool showDebugLights_ = false;
        bool turnOnShadows_ = false;

        int shadowMapSize_ = 1024;

        // Binding points for the UBO and SSBO.
        static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
    };

} // namespace Scene