#include "Scene.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

Scene::Scene()
    : m_PostProcessingEffect(PostProcessingEffectType::None)
{
    m_Camera = std::make_shared<Camera>();

    // FrameData UBO
    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW);

    // Lights SSBO
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW);
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Default
    m_LODEvaluator = std::make_shared<LODEvaluator>();
    m_FrustumCuller = std::make_shared<FrustumCuller>();
}

Scene::~Scene() {}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

void Scene::AddStaticNode(const std::shared_ptr<SceneNode>& node)
{
    m_StaticNodes.push_back(node);
    m_StaticBatchesDirty = true;
}

void Scene::AddDynamicNode(const std::shared_ptr<SceneNode>& node)
{
    m_DynamicNodes.push_back(node);
    // We do not do static batching for these
}

void Scene::AddLight(const LightData& light)
{
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData()
{
    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights.");
    }
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->SetData(m_LightsData.data(),
        m_LightsData.size() * sizeof(LightData),
        sizeof(glm::vec4));
}

void Scene::BindLightSSBO() const
{
    m_LightsSSBO->Bind();
}

void Scene::BuildStaticBatchesIfNeeded()
{
    if (!m_StaticBatchesDirty) return;

    m_StaticBatchManager.Clear();
    // Collect all static render objects from the static scene nodes
    BuildStaticBatchNodes();
    m_StaticBatchManager.BuildBatches();

    m_StaticBatchesDirty = false;
}

void Scene::BuildStaticBatchNodes()
{
    // BFS or DFS over m_StaticNodes. If each node can have children, you'd recursively traverse them.
    for (auto& rootNode : m_StaticNodes) {
        // Flatten the hierarchy
        std::vector<std::shared_ptr<SceneNode>> stack;
        stack.push_back(rootNode);

        while (!stack.empty()) {
            auto node = stack.back();
            stack.pop_back();

            // compute final transform
            // since these nodes do not have parents in this approach, assume node->GetLocalTransform() is final
            if (auto ro = node->GetRenderObject()) {
                ro->GetTransform()->SetModelMatrix(node->GetLocalTransform());
                m_StaticBatchManager.AddRenderObject(ro);
            }

            for (auto& child : node->GetChildren()) {
                stack.push_back(child);
            }
        }
    }
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const
{
    return m_StaticBatchManager.GetBatches();
}

void Scene::Clear()
{
    m_LightsData.clear();
    m_StaticBatchManager.Clear();
    m_StaticNodes.clear();
    m_DynamicNodes.clear();
    m_BGrid = false;
    m_BDebugLights = false;
    m_StaticBatchesDirty = true;
}

void Scene::UpdateFrameDataUBO() const
{
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

void Scene::BindFrameDataUBO() const
{
    m_FrameDataUBO->Bind();
}

/**
 * CullAndLODUpdate:
 *   1) For each object in the static batch, do a frustum check.
 *      If it's culled, set command.count = 0 in the IndirectBuffer.
 *   2) If not culled, set the correct LOD offset.
 */
void Scene::CullAndLODUpdate()
{
    // 1) Early out if no camera or no batches
    if (!m_Camera || !m_LODEvaluator || !m_FrustumCuller) {
        return;
    }

    // 2) First, run the LODEvaluator to pick LOD (like your BatchManager::UpdateLODs logic).
    //    That code modifies the IndirectBuffer commands for LOD. 
    m_StaticBatchManager.UpdateLODs(m_Camera, *m_LODEvaluator);

    // 3) Then do frustum culling. 
    //    We'll do a second pass over the same objects to cull them (by setting count=0).
    //    We can adapt the existing approach: for each object, if culled => count=0.
    //    We can do a minimal approach:
    auto& allBatches = m_StaticBatchManager.GetBatches();
    for (auto& batch : allBatches) {
        const auto& ros = batch->GetRenderObjects();
        for (size_t i = 0; i < ros.size(); i++) {
            auto ro = ros[i];
            glm::vec3 center = ro->GetWorldCenter();
            float radius = ro->GetBoundingSphereRadius();

            bool visible = m_FrustumCuller->IsSphereVisible(center, radius, m_Camera);
            if (!visible) {
                // set command.count=0 in IndirectBuffer
                batch->UpdateLOD(i, /*some invalid LOD or zero count*/ 9999999);
                // A trick: we can set count=0 directly if we add a new method for culling
                // But let's just define a separate method "Batch::SetCountZero(objectIndex)"
                // For example:
                // batch->SetDrawCount(i, 0);
            }
        }
    }
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
{
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const
{
    return m_PostProcessingEffect;
}

void Scene::SetTerrainHeightMap(const std::shared_ptr<ITexture>& heightMap)
{
    m_TerrainHeightMap = heightMap;
}

std::shared_ptr<ITexture> Scene::GetTerrainHeightMap() const
{
    return m_TerrainHeightMap;
}