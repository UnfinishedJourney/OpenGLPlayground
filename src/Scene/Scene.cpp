#include "Scene.h"
#include "Utilities/Logger.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Mesh.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Materials/Material.h"

#include <glad/glad.h>
#include <stdexcept>
#include <glm/glm.hpp>

struct FrameCommonData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;
static const size_t MAX_LIGHTS = 32;

Scene::Scene()
{
    // Default camera
    m_Camera = std::make_shared<Camera>();

    // Setup the UBO for frame data
    m_FrameDataUBO = std::make_unique<UniformBuffer>(
        sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW
    );

    // Setup the Lights SSBO
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(
        LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW
    );
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // LOD + Culling
    m_LODEvaluator = std::make_shared<LODEvaluator>();
    m_FrustumCuller = std::make_shared<FrustumCuller>();
}

Scene::~Scene()
{
    Clear();
}

void Scene::Clear()
{
    m_LightsData.clear();
    m_StaticBatchManager.Clear();
    m_SceneGraph = SceneGraph(); // re-init
    m_LoadedMeshes.clear();
    m_LoadedMaterials.clear();
    m_NodeToRenderObjects.clear();
    m_LastShader.clear();
    m_StaticBatchesDirty = true;
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

bool Scene::LoadModelIntoScene(const std::string& modelName,
    const std::string& defaultShaderName,
    const std::string& defaultMaterialName)
{
    auto& resourceManager = ResourceManager::GetInstance();

    // The ResourceManager knows how to derive mesh/material layout from the shader name
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);
    m_MeshLayout = meshLayout;  // Remember the layout used for future references

    // Use our "BetterModelLoader" to load the model into the SceneGraph
    ModelLoader loader;
    std::string modelPath = ModelLoader::GetModelPath(modelName);
    if (modelPath.empty()) {
        Logger::GetLogger()->error("Unknown model name '{}'. Check your path registry.", modelName);
        return false;
    }

    if (!loader.LoadModel(modelPath, meshLayout, matLayout, m_SceneGraph, true)) {
        Logger::GetLogger()->error("Failed to load model '{}'.", modelName);
        return false;
    }

    // Extract the final loaded data from the loader
    auto& data = loader.GetModelData();

    // Transfer mesh data
    for (auto& md : data.meshesData) {
        MeshInfo info;
        info.mesh = md.mesh;
        info.materialIndex = 0; // or 0 by default, if you want real indexing
        m_LoadedMeshes.push_back(info);
    }

    // Keep track of created materials
    for (auto& matName : data.createdMaterials) {
        m_LoadedMaterials.push_back(matName);
    }

    // The "defaultMaterialName" might be an override, but you're already
    // producing materials in the loader. We'll just store the last used shader name:
    m_LastShader = defaultShaderName;
    m_StaticBatchesDirty = true;

    // Resize the node -> RenderObjects vector
    m_NodeToRenderObjects.resize(m_SceneGraph.GetNodes().size());
    return true;
}

void Scene::AddLight(const LightData& light)
{
    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Max number of lights ({}) reached!", MAX_LIGHTS);
        return;
    }
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData()
{
    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights in the scene!");
    }
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    // first 16 bytes used for numLights (vec4)
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);

    // then the array of LightData
    m_LightsSSBO->SetData(
        m_LightsData.data(),
        m_LightsData.size() * sizeof(LightData),
        sizeof(glm::vec4)
    );
}

void Scene::BindLightSSBO() const
{
    m_LightsSSBO->Bind();
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
{
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const
{
    return m_PostProcessingEffect;
}

void Scene::BuildStaticBatchesIfNeeded()
{
    if (!m_StaticBatchesDirty) {
        return;
    }
    m_StaticBatchesDirty = false;

    // 1) Recompute all global transforms
    m_SceneGraph.RecalculateGlobalTransforms();

    auto& nodes = m_SceneGraph.GetNodes();
    // (Re-)build the node -> RenderObjects array
    if (m_NodeToRenderObjects.size() < nodes.size()) {
        m_NodeToRenderObjects.resize(nodes.size());
    }

    // We’ll clear the old references
    for (auto& roList : m_NodeToRenderObjects) {
        roList.clear();
    }
    m_StaticBatchManager.Clear();

    // 2) For each node, create a RenderObject for each mesh reference
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto& node = nodes[i];
        for (size_t k = 0; k < node.meshIndices.size(); k++) {
            int meshIdx = node.meshIndices[k];
            int matIdx = node.materialIndices[k];

            if (meshIdx < 0 || meshIdx >= (int)m_LoadedMeshes.size()) {
                Logger::GetLogger()->error("Invalid mesh index {} for node {}", meshIdx, i);
                continue;
            }
            // If matIdx out of range, use fallback?
            if (matIdx < 0 || matIdx >= (int)m_LoadedMaterials.size()) {
                Logger::GetLogger()->warn("Material index {} out of range for node {}", matIdx, i);
                matIdx = 0;
            }
            auto& meshInfo = m_LoadedMeshes[meshIdx];
            auto meshPtr = meshInfo.mesh;
            std::string matName = m_LoadedMaterials[matIdx];

            auto transform = std::make_shared<Transform>();
            transform->SetModelMatrix(node.globalTransform);

            auto ro = std::make_shared<RenderObject>(
                meshPtr,
                m_MeshLayout,
                matName,
                m_LastShader,
                transform
            );
            // You could set bounding volumes or other data on ro here

            m_NodeToRenderObjects[i].push_back(ro);
            m_StaticBatchManager.AddRenderObject(ro);
        }
    }

    // 3) Let the BatchManager build final GPU batches
    m_StaticBatchManager.BuildBatches();
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const
{
    return m_StaticBatchManager.GetBatches();
}

void Scene::UpdateFrameDataUBO() const
{
    FrameCommonData data{};
    if (m_Camera) {
        data.view = m_Camera->GetViewMatrix();
        data.proj = m_Camera->GetProjectionMatrix();
        data.cameraPos = glm::vec4(m_Camera->GetPosition(), 1.f);
    }
    else {
        data.view = glm::mat4(1.f);
        data.proj = glm::mat4(1.f);
        data.cameraPos = glm::vec4(0.f, 0.f, 0.f, 1.f);
    }
    m_FrameDataUBO->SetData(&data, sizeof(FrameCommonData));
}

void Scene::BindFrameDataUBO() const
{
    m_FrameDataUBO->Bind();
}

void Scene::CullAndLODUpdate()
{
    if (!m_Camera) {
        return;
    }

    // 1) Construct the view-projection matrix
    glm::mat4 VP = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    m_FrustumCuller->ExtractFrustumPlanes(VP);

    // Example: If you wanted hierarchical culling, you could DFS the SceneGraph here,
    // skipping entire subtrees if the bounding sphere is out.
    // For brevity, we skip that code or you can adapt it from your commented logic.

    // 2) LOD updates (distance-based). For each RenderObject, decide LOD
    //    We let the BatchManager handle the updates, but first we gather them.
    m_StaticBatchManager.UpdateLODs(m_Camera, *m_LODEvaluator);

    // 3) (Optional) Per-object culling if you'd like:
    //    - Possibly call m_StaticBatchManager.CullObject(ro) if out of frustum.
    //    - You can do that in a DFS or a top-level loop.
}