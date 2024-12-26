#include "Scene.h"
#include "Utilities/Logger.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Mesh.h"
#include "Resources/MaterialManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Materials/Material.h"
#include <glad/glad.h>
#include <stdexcept>
#include <glm/glm.hpp>

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;
static const size_t MAX_LIGHTS = 32;

Scene::Scene() {
    m_Camera = std::make_shared<Camera>();
    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW);

    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW);
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_LODEvaluator = std::make_shared<LODEvaluator>();
    m_FrustumCuller = std::make_shared<FrustumCuller>();
}

Scene::~Scene() {
    Clear();
}

void Scene::Clear() {
    m_LightsData.clear();
    m_StaticBatchManager.Clear();
    m_SceneGraph = SceneGraph();
    m_LoadedMeshes.clear();
    m_LoadedMaterials.clear();
    m_NodeToRenderObjects.clear();
    m_StaticBatchesDirty = true;
    m_LastShader.clear();
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera) {
    m_Camera = camera;
}

bool Scene::LoadModelIntoScene(const std::string& modelName,
    const std::string& defaultShaderName,
    const std::string& defaultMaterialName)
{
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);
    m_MeshLayout = meshLayout;

    BetterModelLoader loader;
    std::string modelPath = BetterModelLoader::GetModelPath(modelName);
    if (!loader.LoadModel(modelPath, meshLayout, matLayout, true, m_SceneGraph)) {
        Logger::GetLogger()->error("Failed to load model {}", modelName);
        return false;
    }

    // Retrieve loaded data
    auto& data = loader.GetModelData();
    for (auto& md : data.meshesData) {
        MeshInfo info;
        info.mesh = md.mesh;
        // The loader sets materialName in md.materialName. We store an integer index
        // For now, just store 0 or find your real index
        info.materialIndex = 0;
        m_LoadedMeshes.push_back(info);
    }

    // Also store created material names
    for (auto& matName : data.createdMaterials) {
        m_LoadedMaterials.push_back(matName);
    }

    m_LastShader = defaultShaderName;
    m_StaticBatchesDirty = true;

    // Resize node -> RenderObjects
    m_NodeToRenderObjects.resize(m_SceneGraph.GetNodes().size());

    return true;
}

void Scene::AddLight(const LightData& light) {
    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Max lights reached!");
        return;
    }
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData() {
    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights!");
    }
    uint32_t numLights = (uint32_t)m_LightsData.size();
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->SetData(m_LightsData.data(),
        m_LightsData.size() * sizeof(LightData),
        sizeof(glm::vec4));
}

void Scene::BindLightSSBO() const {
    m_LightsSSBO->Bind();
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect) {
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const {
    return m_PostProcessingEffect;
}

void Scene::BuildStaticBatchesIfNeeded() {
    if (!m_StaticBatchesDirty) return;
    m_StaticBatchesDirty = false;

    m_SceneGraph.RecalculateGlobalTransforms();
    auto& nodes = m_SceneGraph.GetNodes();

    // For each node, create RenderObjects
    for (size_t i = 0; i < nodes.size(); i++) {
        auto& node = nodes[i];
        for (size_t k = 0; k < node.meshIndices.size(); k++) {
            int meshIdx = node.meshIndices[k];
            int matIdx = node.materialIndices[k];

            std::shared_ptr<Mesh> meshPtr = m_LoadedMeshes[meshIdx].mesh;
            std::string matName = "UnknownMaterial";
            if (matIdx >= 0 && matIdx < (int)m_LoadedMaterials.size()) {
                matName = m_LoadedMaterials[matIdx];
            }

            auto transform = std::make_shared<Transform>();
            transform->SetModelMatrix(node.globalTransform);

            auto ro = std::make_shared<RenderObject>(
                meshPtr,
                m_MeshLayout,
                matName,
                m_LastShader,
                transform
            );

            // Keep track of it
            m_NodeToRenderObjects[i].push_back(ro);

            // Add to batch manager
            m_StaticBatchManager.AddRenderObject(ro);
        }
    }

    m_StaticBatchManager.BuildBatches();
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const {
    return m_StaticBatchManager.GetBatches();
}

void Scene::UpdateFrameDataUBO() const {
    FrameCommonData data;
    if (m_Camera) {
        data.view = m_Camera->GetViewMatrix();
        data.proj = m_Camera->GetProjectionMatrix();
        data.cameraPos = glm::vec4(m_Camera->GetPosition(), 1.f);
    }
    else {
        data.view = glm::mat4(1.f);
        data.proj = glm::mat4(1.f);
        data.cameraPos = glm::vec4(0.f);
    }
    m_FrameDataUBO->SetData(&data, sizeof(FrameCommonData));
}

void Scene::BindFrameDataUBO() const {
    m_FrameDataUBO->Bind();
}

void Scene::CullAndLODUpdate() {
    if (!m_Camera) return;

    // 1) Extract planes
    glm::mat4 VP = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    m_FrustumCuller->ExtractFrustumPlanes(VP);

    auto& nodes = m_SceneGraph.GetNodes();

    // Hierarchical culling DFS
    auto cullVisitor = [&](int nodeIndex) -> bool {
        auto& node = nodes[nodeIndex];
        glm::vec3 centerWorld = glm::vec3(node.globalTransform * glm::vec4(node.boundingSphereCenter, 1.f));
        float radius = node.boundingSphereRadius;

        bool visible = m_FrustumCuller->IsSphereVisible(centerWorld, radius);
        if (!visible) {
            // Cull all objects in this node
            for (auto& ro : m_NodeToRenderObjects[nodeIndex]) {
                // Cull in batch
                m_StaticBatchManager.CullObject(ro);
            }
            return false; // skip children
        }

        // If node is visible, we might do per-object culling too
        for (auto& ro : m_NodeToRenderObjects[nodeIndex]) {
            glm::vec3 roCenter = ro->GetWorldCenter();
            float roRadius = ro->GetBoundingSphereRadius();
            if (!m_FrustumCuller->IsSphereVisible(roCenter, roRadius)) {
                m_StaticBatchManager.CullObject(ro);
            }
        }
        return true; // proceed to children
        };

    m_SceneGraph.TraverseGraphDFS(cullVisitor);

    // 2) Now do LOD updates
    m_StaticBatchManager.UpdateLODs(m_Camera, *m_LODEvaluator);
}