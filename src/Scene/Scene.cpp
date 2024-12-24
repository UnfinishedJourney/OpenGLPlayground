#include "Scene.h"
#include "Utilities/Logger.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Mesh.h"
#include "Resources/MaterialManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Materials/Material.h"
#include <glad/glad.h>
#include <stdexcept>

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;
static const size_t MAX_LIGHTS = 32;

Scene::Scene()
    : m_PostProcessingEffect(PostProcessingEffectType::None)
{
    m_Camera = std::make_shared<Camera>();
    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), FRAME_DATA_BINDING_POINT, GL_DYNAMIC_DRAW);

    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(LIGHTS_DATA_BINDING_POINT, bufferSize, GL_DYNAMIC_DRAW);
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_LODEvaluator = std::make_shared<LODEvaluator>();
    m_FrustumCuller = std::make_shared<FrustumCuller>();
}

Scene::~Scene() {
    Clear();
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera) {
    m_Camera = camera;
}

//bool Scene::LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName) {
//    std::string modelPath = ModelLoader::GetModelPath(modelName);
//    if (modelPath.empty()) {
//        Logger::GetLogger()->error("Model '{}' path not found.", modelName);
//        return false;
//    }
//
//    auto& resourceManager = ResourceManager::GetInstance();
//    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);
//    m_MeshLayout = meshLayout;
//    Model model(modelPath, true, meshLayout);
//
//    ModelLoader2 loader2;
//    if (!loader2.LoadModel(modelName, meshLayout, matLayout)) {
//        Logger::GetLogger()->error("Failed to load model '{}' into scene graph.", modelName);
//        return false;
//    }
//
//
//    ModelLoader loader;
//    //if (!loader.LoadIntoSceneGraph(model, meshLayout, matLayout, m_SceneGraph, m_LoadedMeshes, m_LoadedMaterials)) {
//    if (!loader.LoadModelIntoSceneGraph(model, m_SceneGraph, m_LoadedMeshes, m_LoadedMaterials)) {
//        Logger::GetLogger()->error("Failed to load model '{}' into scene graph.", modelName);
//        return false;
//    }
//
//    // Since we rely on standard materials now, ensure they're initialized:
//    resourceManager.GetMaterialManager().InitializeStandardMaterials();
//
//    auto& matManager = MaterialManager::GetInstance();
//    for (const auto& matName : m_LoadedMaterials) {
//        if (!matManager.GetMaterial(matName)) {
//            // Material not found, create a fallback material.
//            // For example, create a Gold-like material but give it the model's material name.
//            glm::vec3 goldAmbient(0.24725f, 0.1995f, 0.0745f);
//            glm::vec3 goldDiffuse(0.75164f, 0.60648f, 0.22648f);
//            glm::vec3 goldSpecular(0.628281f, 0.555802f, 0.366065f);
//            float goldShininess = 51.2f;
//
//            auto fallbackMaterial = matManager.CreateMaterial(matName, goldAmbient, goldDiffuse, goldSpecular, goldShininess);
//            matManager.AddMaterial(matName, fallbackMaterial);
//        }
//    }
//
//    // The defaultMaterialName should now refer to a material that exists in the MaterialManager.
//    // If it doesn't, log a warning or fallback to a known standard material.
//    if (!resourceManager.GetMaterialManager().GetMaterial(defaultMaterialName)) {
//        Logger::GetLogger()->warn("Default material '{}' not found. Using 'Gold' instead.", defaultMaterialName);
//        // Optionally fallback:
//        // defaultMaterialName = "Gold";
//    }
//
//    m_LastShader = defaultShaderName;
//    m_StaticBatchesDirty = true;
//    return true;
//}


bool Scene::LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName) {
    std::string modelPath = ModelLoader::GetModelPath(modelName);
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);
    m_MeshLayout = meshLayout;
    Model model(modelPath, true, meshLayout);
    // 2) Load into SceneGraph
    ModelLoader loader;
    if (!loader.LoadModelIntoSceneGraph(model, m_SceneGraph, m_LoadedMeshes, m_LoadedMaterials)) {
        return false;
    }

    auto& matManager = MaterialManager::GetInstance();

    for (const auto& matName : m_LoadedMaterials) {
        if (!matManager.GetMaterial(matName)) {
            // Material not found, create a fallback material.
            // For example, create a Gold-like material but give it the model's material name.
            glm::vec3 goldAmbient(0.24725f, 0.1995f, 0.0745f);
            glm::vec3 goldDiffuse(0.75164f, 0.60648f, 0.22648f);
            glm::vec3 goldSpecular(0.628281f, 0.555802f, 0.366065f);
            float goldShininess = 51.2f;

            auto fallbackMaterial = matManager.CreateMaterial(matName, goldAmbient, goldDiffuse, goldSpecular, goldShininess);
            matManager.AddMaterial(matName, fallbackMaterial);
        }
    }


    m_LastShader = defaultShaderName;
    m_StaticBatchesDirty = true;
    // 3) Now m_SceneGraph has nodes, bounding volumes, transforms, etc.
    return true;
}


void Scene::AddLight(const LightData& light) {
    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Maximum number of lights reached. Cannot add more.");
        return;
    }
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData() {
    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights.");
    }
    uint32_t numLights = (uint32_t)m_LightsData.size();
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->SetData(m_LightsData.data(), m_LightsData.size() * sizeof(LightData), sizeof(glm::vec4));
}


void Scene::BindLightSSBO() const {
    m_LightsSSBO->Bind();
}

void Scene::BuildStaticBatchesIfNeeded() {

    if (!m_StaticBatchesDirty) return;
    m_StaticBatchesDirty = false;

    auto& nodes = m_SceneGraph.GetNodes();
    for (size_t i = 0; i < nodes.size(); i++) {
        auto& node = nodes[i];
        for (size_t k = 0; k < node.meshIndices.size(); k++) {
            int meshIdx = node.meshIndices[k];
            int materialIdx = node.materialIndices[k];

            // Create a RenderObject
            std::shared_ptr<Mesh> meshPtr = m_LoadedMeshes[meshIdx].mesh;
            std::string materialName = (materialIdx >= 0 &&
                materialIdx < (int)m_LoadedMaterials.size())
                ? m_LoadedMaterials[materialIdx]
                : "UnknownMaterial";

            // For static geometry, store the node's transform in a new Transform?
            // OR we just do "transform->SetModelMatrix(node.globalTransform)".
            auto transform = std::make_shared<Transform>();
            transform->SetModelMatrix(node.globalTransform);

            // Build the RenderObject
            auto ro = std::make_shared<RenderObject>(
                meshPtr,
                /*the same meshLayout*/ m_MeshLayout,
                materialName,
                m_LastShader,
                transform
            );
            // Add it to the batch manager
            m_StaticBatchManager.AddRenderObject(ro);
        }
    }

    m_StaticBatchManager.BuildBatches();
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const {
    return m_StaticBatchManager.GetBatches();
}

void Scene::Clear() {
    m_LightsData.clear();
    m_StaticBatchManager.Clear();
    m_SceneGraph = SceneGraph();
    m_LoadedMeshes.clear();
    m_LoadedMaterials.clear();
    m_BGrid = false;
    m_BDebugLights = false;
    m_StaticBatchesDirty = true;
    m_LastShader.clear();
}

void Scene::UpdateFrameDataUBO() const {
    FrameCommonData frameData;
    if (m_Camera) {
        frameData.view = m_Camera->GetViewMatrix();
        frameData.proj = m_Camera->GetProjectionMatrix();
        frameData.cameraPos = glm::vec4(m_Camera->GetPosition(), 1.0f);
    }
    else {
        frameData.view = glm::mat4(1.0f);
        frameData.proj = glm::mat4(1.0f);
        frameData.cameraPos = glm::vec4(0.0f);
    }
    m_FrameDataUBO->SetData(&frameData, sizeof(FrameCommonData));
}

void Scene::BindFrameDataUBO() const {
    m_FrameDataUBO->Bind();
}

void Scene::CullAndLODUpdate() {
    auto& batches = m_StaticBatchManager.GetBatches();
    for (auto& batch : batches) {
        auto& objects = batch->GetRenderObjects();
        for (size_t i = 0; i < objects.size(); i++) {
            auto ro = objects[i];
            // bounding-sphere cull or bounding-box cull
            bool visible = true; // do your cull check here
            if (!visible) {
                //batch->CullObject(i); // set count=0 in draw command
                continue;
            }
            // LOD
            float distance = 0.0f; // compute distance to camera
            size_t newLOD = 0;     // decide based on thresholds
            ro->SetLOD(newLOD);
            batch->UpdateLOD(i, newLOD);
        }
    }
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect) {
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const {
    return m_PostProcessingEffect;
}