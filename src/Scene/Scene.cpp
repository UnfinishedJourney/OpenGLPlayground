#include "Scene.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/StaticModelLoader.h"
#include "Renderer/RenderObject.h"

struct FrameCommonData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;

Scene::Scene()
{
    m_SceneGraph = std::make_unique<SceneGraph>();
    m_Camera = std::make_shared<Camera>();

    // Create frame UBO
    m_FrameDataUBO = std::make_unique<UniformBuffer>(
        sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW
    );

    m_LODEvaluator = std::make_unique<LODEvaluator>();
    m_FrustumCuller = std::make_unique<FrustumCuller>();
    m_StaticBatchManager = std::make_unique<BatchManager>();

    // Create a shared LightManager
    m_LightManager = std::make_shared<LightManager>();
}

Scene::~Scene()
{
    Clear();
}

void Scene::Clear()
{
    // Clear geometry
    if (m_StaticBatchManager)
        m_StaticBatchManager->Clear();
    m_StaticObjects.clear();
    m_StaticBatchesDirty = true;

    // Optionally re-init the light manager to empty
    m_LightManager = std::make_shared<LightManager>();

    // Re-init scene graph
    m_SceneGraph = std::make_unique<SceneGraph>();

    // Not clearing the camera because you might keep the same pointer
    // or reassign it if you wish
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

bool Scene::LoadStaticModelIntoScene(const std::string& modelName,
    const std::string& shaderName,
    float scaleFactor)
{
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(shaderName);

    staticloader::ModelLoader loader(scaleFactor);
    bool success = loader.LoadStaticModel(modelName, meshLayout, matLayout, /*centerModel=*/true);
    if (!success) {
        Logger::GetLogger()->error("Failed to load static model '{}'.", modelName);
        return false;
    }

    const auto& objects = loader.GetLoadedObjects();
    const auto& materials = loader.GetLoadedMaterials();

    for (auto& mi : objects) {
        auto ro = std::make_shared<StaticRenderObject>(
            mi.mesh,
            meshLayout,
            mi.materialIndex,
            shaderName
        );
        m_StaticObjects.push_back(ro);
    }

    m_LastShaderName = shaderName;
    m_StaticBatchesDirty = true;

    Logger::GetLogger()->info("Loaded static model '{}' => {} sub-meshes, {} materials.",
        modelName, objects.size(), materials.size());
    return true;
}

bool Scene::LoadPrimitiveIntoScene(const std::string& primitiveName,
    const std::string& shaderName,
    int materialID)
{
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(shaderName);

    auto mesh = MeshManager::GetInstance().GetMesh(primitiveName);
    if (!mesh) {
        Logger::GetLogger()->error("Primitive '{}' not found in MeshManager!", primitiveName);
        return false;
    }

    auto ro = std::make_shared<StaticRenderObject>(
        mesh,
        meshLayout,
        materialID,
        shaderName
    );

    m_StaticObjects.push_back(ro);
    m_StaticBatchesDirty = true;
    Logger::GetLogger()->info("Scene::LoadPrimitiveIntoScene: Loaded '{}' with shader '{}'.",
        primitiveName, shaderName);
    return true;
}

void Scene::BuildStaticBatchesIfNeeded()
{
    if (!m_StaticBatchesDirty)
        return;

    m_StaticBatchesDirty = false;
    if (m_StaticBatchManager) {
        m_StaticBatchManager->Clear();
        for (auto& ro : m_StaticObjects) {
            m_StaticBatchManager->AddRenderObject(ro);
        }
        m_StaticBatchManager->BuildBatches();
    }
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const
{
    return m_StaticBatchManager->GetBatches();
}

void Scene::UpdateFrameDataUBO() const
{
    if (!m_Camera) return;

    FrameCommonData data{};
    data.view = m_Camera->GetViewMatrix();
    data.proj = m_Camera->GetProjectionMatrix();
    data.cameraPos = glm::vec4(m_Camera->GetPosition(), 1.0f);

    m_FrameDataUBO->SetData(&data, sizeof(FrameCommonData));
}

void Scene::BindFrameDataUBO() const
{
    m_FrameDataUBO->Bind();
}

void Scene::CullAndLODUpdate()
{
    if (!m_Camera) return;

    glm::mat4 VP = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    m_FrustumCuller->ExtractFrustumPlanes(VP);

    // optional: cull out-of-frustum
    m_StaticBatchManager->UpdateLODs(m_Camera, *m_LODEvaluator);
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
{
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const
{
    return m_PostProcessingEffect;
}