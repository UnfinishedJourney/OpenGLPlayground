#include "Scene.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Scene/Screen.h"
#include <glad/glad.h>


Scene::Scene()
    : m_PostProcessingShaderName("presentTexture")
{
    auto logger = Logger::GetLogger();
    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), 0, GL_DYNAMIC_DRAW);
    logger->info("Created FrameData UBO with binding point 0.");
    //m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(1, sizeof(LightData) * MAX_LIGHTS, GL_DYNAMIC_DRAW);
    m_LightsData = {
        { glm::vec4(1.5f, 1.5f, 1.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
    };

    GLsizeiptr bufferSize = sizeof(glm::vec4) + m_LightsData.size() * sizeof(LightData);

    // Create and bind the SSBO for lights
    glGenBuffers(1, &m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

    // Initialize numLights and lightsData
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), &numLights);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), m_LightsData.size() * sizeof(LightData), m_LightsData.data());

    // Bind the SSBO to binding point 1
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    logger->info("Initialized Lights SSBO with {} light(s).", m_LightsData.size());
}

Scene::~Scene()
{
    if (m_LightsSSBO)
    {
        glDeleteBuffers(1, &m_LightsSSBO);
    }
}

void Scene::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_BatchManager.AddRenderObject(renderObject);
    //m_RenderObjects.push_back(renderObject);
}

//void Scene::AddLight(const std::shared_ptr<Light>& light)
//{
//    m_Lights.push_back(light);
//}

void Scene::AddLight(LightData light)
{
    m_LightsData.push_back(light);
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

void Scene::Clear()
{
    //m_RenderObjects.clear();
    m_LightsData.clear();
    //m_Lights.clear();
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

void Scene::UpdateLightsData() const
{
    std::vector<LightData> lightDataVec;
    //for (const auto& light : m_Lights) {
    //    LightData data;
    //    data.position = glm::vec4(light->GetPosition(), 1.0f);
    //    data.color = glm::vec4(light->GetColor(), 1.0f);
    //    // Add other light properties if needed
    //    lightDataVec.push_back(data);
    //}

    for (const auto& light : m_LightsData) {
        lightDataVec.push_back(light);
    }
     
    // Update the SSBO
    //m_LightsSSBO->SetData(lightDataVec.data(), lightDataVec.size() * sizeof(LightData));
}

void Scene::BindLightSSBO() const
{
    ///m_LightsSSBO->Bind();
}

void Scene::BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const
{
    ResourceManager::GetInstance().BindShader(shaderName);
    ResourceManager::GetInstance().BindMaterial(materialName);
}