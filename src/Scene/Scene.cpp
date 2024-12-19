#include "Scene.h"
#include "Utilities/Logger.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Mesh.h"
#include "Resources/MaterialManager.h"
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

bool Scene::LoadModelIntoScene(const std::string& modelName, const std::string& defaultShaderName, const std::string& defaultMaterialName, const MeshLayout& meshLayout, const MaterialLayout& matLayout) {
    auto pathIt = m_ModelPaths.find(modelName);
    if (pathIt == m_ModelPaths.end()) {
        Logger::GetLogger()->error("Model '{}' path not found.", modelName);
        return false;
    }

    std::string modelPath = pathIt->second;
    // Load the model
    Model model(modelPath, true, meshLayout);

    ModelLoader loader;
    if (!loader.LoadIntoSceneGraph(model, meshLayout, m_SceneGraph, m_LoadedMeshes, m_LoadedMaterials)) {
        Logger::GetLogger()->error("Failed to load model '{}' into scene graph.", modelName);
        return false;
    }

    // Create materials using MaterialManager
    auto& matManager = MaterialManager::GetInstance();
    for (const auto& matName : m_LoadedMaterials) {
        if (!matManager.GetMaterial(matName)) {
            // Assuming the layout corresponds to all loaded materials
            auto mat = matManager.CreateMaterial("Gold", glm::vec3(0.24725f, 0.1995f, 0.0745f), glm::vec3(0.75164f, 0.60648f, 0.22648f), glm::vec3(0.628281f, 0.555802f, 0.366065f), 51.2f);
            // Adjust parameters as needed based on the material name
            if (matName == "Silver") {
                mat = matManager.CreateMaterial("Silver", glm::vec3(0.19225f, 0.19225f, 0.19225f), glm::vec3(0.50754f, 0.50754f, 0.50754f), glm::vec3(0.508273f, 0.508273f, 0.508273f), 51.2f);
            }

            //goldMaterial->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
            //goldMaterial->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
            //goldMaterial->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
            //goldMaterial->AddParam<float>("material.shininess", 51.2f);
            matManager.AddMaterial(matName, mat);
        }
    }

    // Store defaults for this model if needed
    m_DefaultShader = defaultShaderName;
    m_DefaultMaterial = defaultMaterialName;

    m_StaticBatchesDirty = true;
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

void Scene::buildBatchesFromSceneGraph() {
    m_StaticBatchManager.Clear();

    auto& matManager = MaterialManager::GetInstance();

    const auto& nodes = m_SceneGraph.GetNodes();

    for (const auto& node : nodes) {
        if (node.meshIndex < 0) continue;

        auto meshInfo = m_LoadedMeshes[node.meshIndex];
        auto mesh = meshInfo.mesh;

        auto transform = std::make_shared<Transform>();
        transform->SetModelMatrix(node.globalTransform);

        int matIdx = node.materialIndex;
        std::string matName = m_DefaultMaterial;
        if (matIdx >= 0 && matIdx < static_cast<int>(m_LoadedMaterials.size())) {
            matName = m_LoadedMaterials[matIdx];
        }

        // Use defaultShader or a fallback if not set:
        std::string shaderName = m_DefaultShader.empty() ? "defaultShader" : m_DefaultShader;

        auto ro = std::make_shared<RenderObject>(mesh, MeshLayout{ true, true }, matName, shaderName, transform);
        m_StaticBatchManager.AddRenderObject(ro);
    }
}

void Scene::BuildStaticBatchesIfNeeded() {
    if (!m_StaticBatchesDirty) return;
    buildBatchesFromSceneGraph();
    m_StaticBatchManager.BuildBatches();
    m_StaticBatchesDirty = false;
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
    m_DefaultShader.clear();
    m_DefaultMaterial.clear();
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
    if (!m_Camera || !m_LODEvaluator || !m_FrustumCuller) return;

    m_StaticBatchManager.UpdateLODs(m_Camera, *m_LODEvaluator);

    auto& batches = m_StaticBatchManager.GetBatches();
    for (auto& b : batches) {
        auto& ros = b->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); ++i) {
            auto ro = ros[i];
            glm::vec3 c = ro->GetWorldCenter();
            float r = ro->GetBoundingSphereRadius();
            bool visible = m_FrustumCuller->IsSphereVisible(c, r, m_Camera);
            if (!visible) {
                b->UpdateLOD(i, static_cast<size_t>(-1));
            }
        }
    }
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect) {
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const {
    return m_PostProcessingEffect;
}