#include "Scene.h"
#include "Utilities/Logger.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include <glad/glad.h>

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

Scene::Scene()
    : m_PostProcessingEffect(PostProcessingEffectType::None)
{
    m_Camera = std::make_shared<Camera>();
    auto logger = Logger::GetLogger();

    // Create root node
    m_RootNode = std::make_shared<SceneNode>();

    // FrameData UBO
    m_FrameDataUBO = std::make_unique<UniformBuffer>(
        sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW
    );
    logger->info("Created FrameData UBO with binding point {}.", FRAME_DATA_BINDING_POINT);

    // Lights SSBO
    GLuint bindingPoint = LIGHTS_DATA_BINDING_POINT;
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;

    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(bindingPoint, bufferSize, GL_DYNAMIC_DRAW);
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    // initial no lights
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    logger->info("Initialized Lights SSBO with 0 lights.");
}

Scene::~Scene()
{
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
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
    m_LightsSSBO->SetData(m_LightsData.data(), m_LightsData.size() * sizeof(LightData), sizeof(glm::vec4));
}

void Scene::BindLightSSBO() const
{
    m_LightsSSBO->Bind();
}

void Scene::Clear()
{
    m_LightsData.clear();
    m_BatchManager.Clear();
    m_TerrainHeightMap = nullptr;
    m_BGrid = false;
    m_BDebugLights = false;

    // Reset root node
    m_RootNode = std::make_shared<SceneNode>();
}

void Scene::BuildBatches()
{
    if (m_bFirstTraverse)
    {
        // 1. Clear old
        m_BatchManager.Clear();

        // 2. Traverse the scene graph, collect all render objects
        TraverseSceneGraph(m_RootNode, glm::mat4(1.0f));

        // 3. Build
        m_BatchManager.BuildBatches();
        m_bFirstTraverse = false;
    }

}

void Scene::TraverseSceneGraph(std::shared_ptr<SceneNode> node, const glm::mat4& parentTransform)
{
    if (!node) return;

    glm::mat4 worldTransform = node->CalculateWorldTransform(parentTransform);

    // If node has a RenderObject, set the final model matrix
    if (auto ro = node->GetRenderObject()) {
        ro->GetTransform()->SetModelMatrix(worldTransform);
        m_BatchManager.AddRenderObject(ro);
    }

    // Recurse children
    for (const auto& child : node->GetChildren()) {
        TraverseSceneGraph(child, worldTransform);
    }
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetBatches() const
{
    return m_BatchManager.GetBatches();
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