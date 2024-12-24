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


bool Scene::LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName) {
    std::string modelPath = ModelLoader::GetModelPath(modelName);
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);
    m_MeshLayout = meshLayout;
    BetterModelLoader loader;
    if (!loader.LoadModel(modelPath, meshLayout, matLayout, true, m_SceneGraph)) {
        // Handle error
        return false;
    }

    auto& data = loader.GetModelData();

    for (size_t i = 0; i < data.meshesData.size(); i++)
    {
        MeshInfo info;
        BetterModelMeshData td = data.meshesData[i];
        info.mesh = td.mesh;
        info.materialIndex = 0; // Or see below on how to set the correct index
        m_LoadedMeshes.push_back(info);
    }

    // Also store the material names
    // e.g., data.materialNames might have N materials
    // For each name, push back to m_LoadedMaterials to maintain your old style
    // or just store them directly in a vector<string> if that’s all you need.
    for (auto& matName : data.createdMaterials) {
        m_LoadedMaterials.push_back(matName);
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