#include "LightManager.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h" // or wherever your SSBO/UBO is
#include <glm/gtc/matrix_transform.hpp>

static constexpr size_t MAX_LIGHTS = 32;

static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

LightManager::LightManager()
{
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
}

LightManager::~LightManager() = default;

//void LightManager::AddLight(const Light& light)
//{
//
//    if (m_LightsData.size() >= MAX_LIGHTS) {
//        Logger::GetLogger()->warn("Scene::AddLight: exceeded MAX_LIGHTS = {}", MAX_LIGHTS);
//        return;
//    }
//
//    m_Lights.push_back(light);
//    UpdateLightsGPU();
//}
//
//const std::vector<Light>& LightManager::GetLights() const
//{
//    return m_Lights;
//}

std::optional<size_t> LightManager::AddLight(const LightData& light)
{

    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Scene::AddLight: exceeded MAX_LIGHTS = {}", MAX_LIGHTS);
        return std::nullopt;
    }

    m_LightsData.push_back(light);
    //maybe should update all lights at once
    UpdateLightsGPU();
    return m_LightsData.size() - 1;
}


void LightManager::UpdateLightsGPU()
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

void LightManager::BindLightsGPU() const
{
    if (m_LightsSSBO) {
        m_LightsSSBO->BindBase();
    }
}

glm::mat4 LightManager::ComputeLightView(size_t id) const
{
    if (id >= m_LightsData.size()) {
        Logger::GetLogger()->warn("Scene::ComputeLightView: wrong index", MAX_LIGHTS);
        return 1.0f;
    }

    const auto& lightData = m_LightsData[id];

    if (lightData.position.w == 0.0)
    {
        return ComputeDirectionalLightView(lightData);
    }
    else
    {
        return ComputePointLightView(lightData);
    }
}


glm::mat4 LightManager::ComputeLightProj(size_t id) const
{
    if (id >= m_LightsData.size()) {
        Logger::GetLogger()->warn("Scene::ComputeLightView: wrong index", MAX_LIGHTS);
        return 1.0f;
    }

    const auto& lightData = m_LightsData[id];

    if (lightData.position.w < 0.5)
    {
        return ComputeDirectionalLightProj(lightData);
    }
    else
    {
        return ComputePointLightProj(lightData);
    }
}

glm::mat4 LightManager::ComputeDirectionalLightView(const LightData& light) const
{
    glm::vec3 pos = light.position;
    glm::vec3 dir = glm::normalize(pos);
    glm::vec3 up = glm::vec3(0, 1, 0);
    // If dir is near (0,1,0), pick a different up to avoid degeneracy
    return glm::lookAt(glm::vec3(0), dir, up);
}

glm::mat4 LightManager::ComputeDirectionalLightProj(const LightData& light) const
{
    // We'll do the standard approach:
    // 1) transform scene bounds into light space
    // 2) build an ortho to enclose that bounding box
    glm::mat4 lightView = ComputeDirectionalLightView(light);

    // gather corners
    glm::vec3 corners[8] = {
        {m_BBox.min_.x, m_BBox.min_.y, m_BBox.min_.z},
        {m_BBox.min_.x, m_BBox.min_.y, m_BBox.max_.z},
        {m_BBox.min_.x, m_BBox.max_.y, m_BBox.min_.z},
        {m_BBox.min_.x, m_BBox.max_.y, m_BBox.max_.z},
        {m_BBox.max_.x, m_BBox.min_.y, m_BBox.min_.z},
        {m_BBox.max_.x, m_BBox.min_.y, m_BBox.max_.z},
        {m_BBox.max_.x, m_BBox.max_.y, m_BBox.min_.z},
        {m_BBox.max_.x, m_BBox.max_.y, m_BBox.max_.z},
    };
    glm::vec3 minLS(1e9f), maxLS(-1e9f);

    for (auto& corner : corners) {
        glm::vec4 t = lightView * glm::vec4(corner, 1.0f);
        minLS = glm::min(minLS, glm::vec3(t));
        maxLS = glm::max(maxLS, glm::vec3(t));
    }

    float left = minLS.x;
    float right = maxLS.x;
    float bottom = minLS.y;
    float top = maxLS.y;
    float zNear = minLS.z;
    float zFar = maxLS.z;

    return glm::ortho(left, right, bottom, top, zNear, zFar);
}

glm::mat4 LightManager::ComputePointLightView(const LightData& light) const
{
    glm::vec3 lightPos = light.position;
    // Pick a target for the light’s view; for instance, the center of the scene:
    glm::vec3 target = glm::vec3(0.0f);
    // Choose a suitable up vector:
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Build the view matrix from the light's point of view:
    glm::mat4 lightView = glm::lookAt(lightPos, target, up);
    
    return lightView;
}

glm::mat4 LightManager::ComputePointLightProj(const LightData& light) const
{
    // For the projection, a 90° FOV works well if we want a symmetric frustum.
    float nearPlane = 1.0f; // Adjust based on scene
    float farPlane = 100.0f; // Adjust based on scene's extent
    glm::mat4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

    return lightProj;
}