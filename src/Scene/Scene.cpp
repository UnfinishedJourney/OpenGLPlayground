#include "Scene.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/StaticModelLoader.h"
#include "Renderer/RenderObject.h"
#include <cfloat>  // For FLT_MAX

namespace Scene {

    /**
     * @brief Structure containing per-frame common data for rendering.
     */
    struct alignas(16) FrameCommonData
    {
        glm::mat4 view_;
        glm::mat4 proj_;
        glm::vec4 cameraPos_;
    };

    Scene::Scene()
    {
        // Initialize the scene graph.
        sceneGraph_ = std::make_unique<SceneGraph>();

        // Create a default camera.
        camera_ = std::make_shared<Camera>();

        // Create the per-frame UBO using a Std140 layout.
        frameDataUBO_ = std::make_unique<graphics::UniformBuffer>(
            sizeof(FrameCommonData),
            FRAME_DATA_BINDING_POINT,
            graphics::UniformBufferLayout::Std140,
            GL_DYNAMIC_DRAW
        );

        // Initialize LOD evaluator and frustum culler.
        lodEvaluator_ = std::make_unique<LODEvaluator>();
        frustumCuller_ = std::make_unique<FrustumCuller>();

        // Create the static batch manager.
        staticBatchManager_ = std::make_unique<BatchManager>();

        // Create a shared light manager.
        lightManager_ = std::make_shared<LightManager>();

        // The object data SSBO will be created when static objects are added.
        objectDataSSBO_ = nullptr;
    }

    Scene::~Scene()
    {
        Clear();
    }

    void Scene::Clear()
    {
        Logger::GetLogger()->info("Clearing scene.");

        // Clear geometry and batches.
        if (staticBatchManager_)
            staticBatchManager_->Clear();
        staticObjects_.clear();
        staticBatchesDirty_ = true;

        // Reinitialize the light manager.
        lightManager_ = std::make_shared<LightManager>();

        // Reinitialize the scene graph.
        sceneGraph_ = std::make_unique<SceneGraph>();
    }

    void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
    {
        if (!camera) {
            Logger::GetLogger()->warn("Attempted to set a null camera.");
            return;
        }
        camera_ = camera;
        Logger::GetLogger()->info("Camera set for the scene.");
    }

    bool Scene::LoadStaticModelIntoScene(const std::string& modelName,
        const std::string& shaderName,
        float scaleFactor,
        std::unordered_map<aiTextureType, TextureType> aiToMyType)
    {
        Logger::GetLogger()->info("Loading static model '{}' with shader '{}'.", modelName, shaderName);

        auto& resourceManager = ResourceManager::GetInstance();
        auto [meshLayout, matLayout] = resourceManager.GetLayoutsFromShader(shaderName);

        // Use the StaticModelLoader to load the model.
        StaticLoader::ModelLoader loader(scaleFactor, aiToMyType);
        bool success = loader.LoadStaticModel(modelName, meshLayout, matLayout, /*centerModel=*/true);
        if (!success) {
            Logger::GetLogger()->error("Failed to load static model '{}'.", modelName);
            return false;
        }

        const auto& loadedObjects = loader.GetLoadedObjects();

        // Create render objects for each sub-mesh.
        for (const auto& meshInfo : loadedObjects) {
            auto renderObj = std::make_shared<StaticRenderObject>(
                meshInfo.mesh_,
                meshLayout,
                meshInfo.materialIndex_,
                shaderName
            );
            staticObjects_.push_back(renderObj);
        }

        lastShaderName_ = shaderName;
        staticBatchesDirty_ = true;

        Logger::GetLogger()->info("Loaded static model '{}' with {} sub-mesh(es).", modelName, loadedObjects.size());
        return true;
    }

    bool Scene::LoadPrimitiveIntoScene(const std::string& primitiveName,
        const std::string& shaderName,
        int materialID)
    {
        Logger::GetLogger()->info("Loading primitive '{}' with shader '{}'.", primitiveName, shaderName);

        auto& resourceManager = ResourceManager::GetInstance();
        auto [meshLayout, matLayout] = resourceManager.GetLayoutsFromShader(shaderName);

        auto mesh = graphics::MeshManager::GetInstance().GetMesh(primitiveName);
        if (!mesh) {
            Logger::GetLogger()->error("Primitive '{}' not found in MeshManager!", primitiveName);
            return false;
        }

        auto renderObj = std::make_shared<StaticRenderObject>(
            mesh,
            meshLayout,
            materialID,
            shaderName
        );

        staticObjects_.push_back(renderObj);
        staticBatchesDirty_ = true;

        Logger::GetLogger()->info("Loaded primitive '{}' successfully.", primitiveName);
        return true;
    }

    void Scene::BuildStaticBatchesIfNeeded()
    {
        if (!staticBatchesDirty_)
            return;

        Logger::GetLogger()->info("Building static batches.");

        staticBatchesDirty_ = false;
        if (staticBatchManager_) {
            staticBatchManager_->Clear();
            for (const auto& renderObj : staticObjects_) {
                staticBatchManager_->AddRenderObject(renderObj);
            }
            staticBatchManager_->BuildBatches();
        }

        // If shadows are enabled, update the light manager's bounding box.
        if (turnOnShadows_) {
            BoundingBox bb = ComputeWorldBoundingBox();
            lightManager_->SetBoundingBox(bb.min_, bb.max_);
        }
    }

    const std::vector<std::shared_ptr<renderer::Batch>>& Scene::GetStaticBatches() const
    {
        return staticBatchManager_->GetBatches();
    }

    void Scene::UpdateFrameDataUBO() const
    {
        if (!camera_) {
            Logger::GetLogger()->warn("No camera available. Skipping frame UBO update.");
            return;
        }

        FrameCommonData frameData{};
        frameData.view_ = camera_->GetViewMatrix();
        frameData.proj_ = camera_->GetProjectionMatrix();
        frameData.cameraPos_ = glm::vec4(camera_->GetPosition(), 1.0f);

        frameDataUBO_->UpdateData(
            { reinterpret_cast<const std::byte*>(&frameData), sizeof(frameData) },
            0
        );
        Logger::GetLogger()->debug("Updated frame data UBO.");
    }

    void Scene::BindFrameDataUBO() const
    {
        frameDataUBO_->Bind();
        Logger::GetLogger()->debug("Bound frame data UBO.");
    }

    void Scene::CullAndLODUpdate()
    {
        if (!camera_) {
            Logger::GetLogger()->warn("No camera available. Skipping culling and LOD update.");
            return;
        }

        glm::mat4 VP = camera_->GetProjectionMatrix() * camera_->GetViewMatrix();
        frustumCuller_->ExtractFrustumPlanes(VP);
        Logger::GetLogger()->debug("Extracted frustum planes.");

        if (staticBatchManager_) {
            staticBatchManager_->UpdateLODs(camera_, *lodEvaluator_);
            Logger::GetLogger()->debug("Updated LODs for static batches.");
        }
    }

    void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
    {
        postProcessingEffect_ = effect;
        Logger::GetLogger()->info("Post-processing effect set to {}.", static_cast<int>(effect));
    }

    PostProcessingEffectType Scene::GetPostProcessingEffect() const
    {
        return postProcessingEffect_;
    }

    BoundingBox Scene::ComputeWorldBoundingBox() const
    {
        BoundingBox worldBox;
        worldBox.min_ = glm::vec3(FLT_MAX);
        worldBox.max_ = glm::vec3(-FLT_MAX);

        for (const auto& renderObj : staticObjects_) {
            if (!renderObj || !renderObj->GetMesh())
                continue;

            glm::vec3 localMin = renderObj->GetMesh()->minBounds_;
            glm::vec3 localMax = renderObj->GetMesh()->maxBounds_;

            worldBox.combinePoint(localMin);
            worldBox.combinePoint(localMax);
        }

        Logger::GetLogger()->debug("Computed world bounding box: min({},{},{}) max({},{},{}).",
            worldBox.min_.x, worldBox.min_.y, worldBox.min_.z,
            worldBox.max_.x, worldBox.max_.y, worldBox.max_.z);
        return worldBox;
    }

} // namespace Scene