#include "Scene.h"

// Include only what's necessary
#include <glad/glad.h>
#include <stdexcept>
#include <algorithm>

// For your uniform struct (could keep this internal or in a header)
#include <glm/gtc/matrix_transform.hpp>

// Local includes
#include "Renderer/BatchManager.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Scene/Camera.h"
#include "Scene/SceneGraph.h"
#include "Scene/Lights.h"
#include "Scene/FrustumCuller.h"
#include "Scene/LODEvaluator.h"
#include "Utilities/Logger.h"
#include "Resources/MaterialManager.h"
#include "Graphics/Meshes/StaticModelLoader.h"
#include "Resources/ResourceManager.h"

// Example for the frame data
struct FrameCommonData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

// Max lights in your scene
static constexpr size_t MAX_LIGHTS = 32;

// Binding points
static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

Scene::Scene()
{
    // Possibly create your scene graph if you still want hierarchical
    m_SceneGraph = std::make_unique<SceneGraph>();

    m_Camera = std::make_shared<Camera>();

    // Create the UBO for frame data
    m_FrameDataUBO = std::make_unique<UniformBuffer>(
        sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW
    );

    // Create the Lights SSBO
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(
        LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW
    );
    // Initialize lights count to zero
    uint32_t numLights = 0;
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase(); // optional

    // Setup LOD/culling
    m_LODEvaluator = std::make_unique<LODEvaluator>();
    m_FrustumCuller = std::make_unique<FrustumCuller>();

    // Batch manager
    m_StaticBatchManager = std::make_unique<BatchManager>();
}

Scene::~Scene()
{
    Clear();
}

void Scene::Clear()
{
    // Clear any stored objects, lights, etc.
    m_LightsData.clear();

    if (m_StaticBatchManager) {
        m_StaticBatchManager->Clear();
    }

    // If you keep a scene graph:
    m_SceneGraph = std::make_unique<SceneGraph>(); // or reset if you want

    m_StaticObjects.clear();
    m_StaticBatchesDirty = true;
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

void Scene::AddLight(const LightData& light)
{
    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Scene::AddLight: exceeded MAX_LIGHTS = {}", MAX_LIGHTS);
        return;
    }
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData()
{
    // Number of lights is the first vec4
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);

    // Then the array of LightData
    if (!m_LightsData.empty()) {
        size_t dataSize = m_LightsData.size() * sizeof(LightData);
        m_LightsSSBO->SetData(m_LightsData.data(), dataSize, sizeof(glm::vec4));
    }
}

void Scene::BindLightSSBO() const
{
    // Usually you call BindBase() once, but if you want to 
    // ensure it's bound, you can call:
    m_LightsSSBO->BindBase();
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
{
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const
{
    return m_PostProcessingEffect;
}

//------------------------------------------------------------------------------------
//  Static model loading
//------------------------------------------------------------------------------------
bool Scene::LoadStaticModelIntoScene(const std::string& modelName,
    const std::string& shaderName,
    float scaleFactor,
    std::unordered_map<aiTextureType, TextureType> aiToMyType)
{
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(shaderName);

    staticloader::ModelLoader loader(scaleFactor, aiToMyType);
    bool success = loader.LoadStaticModel(modelName, meshLayout, matLayout, /*centerModel=*/true);
    if (!success) {
        Logger::GetLogger()->error("Failed to load static model '{}'.", modelName);
        return false;
    }

    // Retrieve the baked objects from the loader
    const auto& objects = loader.GetLoadedObjects();
    const auto& materials = loader.GetLoadedMaterials();

    for (auto& mi : objects) {
        int materialID = mi.materialIndex;

        // Create a static (baked) render object
        auto ro = std::make_shared<StaticRenderObject>(
            mi.mesh,
            meshLayout,
            materialID,
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
    
    auto renderObject = std::make_shared<StaticRenderObject>(mesh, meshLayout, materialID, shaderName);


    m_StaticObjects.push_back(renderObject);
    m_StaticBatchesDirty = true;

    Logger::GetLogger()->info("Scene::LoadPrimitiveIntoScene: Loaded primitive '{}' with shader '{}'.", primitiveName, shaderName);
    return true;
}


void Scene::BuildStaticBatchesIfNeeded()
{
    if (!m_StaticBatchesDirty) {
        return;
    }
    m_StaticBatchesDirty = false;

    // Clear old data
    m_StaticBatchManager->Clear();

    // If you want to handle hierarchical objects in SceneGraph, do so here
    // For each node, gather meshes -> create RenderObjects
    // Then add them to m_StaticBatchManager

    // Add “static” baked objects
    for (auto& ro : m_StaticObjects) {
        m_StaticBatchManager->AddRenderObject(ro);
    }

    m_StaticBatchManager->BuildBatches();
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const
{
    return m_StaticBatchManager->GetBatches();
}

//------------------------------------------------------------------------------------
// Per-frame uniform updates
//------------------------------------------------------------------------------------
void Scene::UpdateFrameDataUBO() const
{
    if (!m_Camera) {
        return;
    }
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

//------------------------------------------------------------------------------------
// Culling & LOD
//------------------------------------------------------------------------------------
void Scene::CullAndLODUpdate()
{
    if (!m_Camera) {
        return;
    }

    // Extract frustum planes
    glm::mat4 VP = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    m_FrustumCuller->ExtractFrustumPlanes(VP);

    // Possibly cull out-of-frustum objects
    // (Currently disabled if you do not need culling.)

    // Evaluate LOD for each object in the batch manager
    m_StaticBatchManager->UpdateLODs(m_Camera, *m_LODEvaluator);
}