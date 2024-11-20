#include "Scene.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Scene/Screen.h"
#include <glad/glad.h>

constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

Scene::Scene()
    : m_PostProcessingEffect(PostProcessingEffectType::None)
{
    m_Camera = std::make_shared<Camera>();
    auto logger = Logger::GetLogger();
    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), FRAME_DATA_BINDING_POINT, GL_DYNAMIC_DRAW);
    logger->info("Created FrameData UBO with binding point {}.", FRAME_DATA_BINDING_POINT);
    m_LightsData = {};

    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights.");
    }

    GLuint bindingPoint = LIGHTS_DATA_BINDING_POINT;
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());

    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(bindingPoint, bufferSize, GL_DYNAMIC_DRAW);
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->SetData(m_LightsData.data(), m_LightsData.size() * sizeof(LightData), sizeof(glm::vec4));

    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    logger->info("Initialized Lights SSBO with {} light(s).", m_LightsData.size());
}

Scene::~Scene()
{
}

void Scene::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_BatchManager.AddRenderObject(renderObject);
}

void Scene::AddLight(const LightData& light)
{
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

void Scene::Clear()
{
    m_LightsData.clear();
    m_BatchManager.Clear();
}

void Scene::BuildBatches() const
{
    m_BatchManager.BuildBatches();
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

void Scene::BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const
{
    auto& shaderManager = ShaderManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();

    auto shader = shaderManager.GetShader(shaderName);
    if (shader) {
        shader->Bind();
    }
    else {
        Logger::GetLogger()->error("Shader '{}' not found.", shaderName);
        return;
    }

    auto material = materialManager.GetMaterial(materialName);
    if (material) {
        materialManager.BindMaterial(materialName, shader);
    }
    else {
        Logger::GetLogger()->error("Material '{}' not found.", materialName);
        return;
    }

    // Bind frame data UBO and light SSBO
    BindFrameDataUBO();
    BindLightSSBO();
}

void Scene::SetTerrainHeightMap(const std::shared_ptr<Texture2D>& heightMap)
{
    m_TerrainHeightMap = heightMap;
}

std::shared_ptr<Texture2D> Scene::GetTerrainHeightMap() const
{
    return m_TerrainHeightMap;
}