#include "LightManager.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h" // or wherever your SSBO/UBO is
#include <glm/gtc/matrix_transform.hpp>
#include "Utilities/Logger.h"
static constexpr size_t MAX_LIGHTS = 32;

static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

LightManager::LightManager()
{
    // Create the Lights SSBO
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    lightsSSBO_ = std::make_unique<Graphics::ShaderStorageBuffer>(
        LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW
    );

    // Initialize lights count to zero
    std::array<uint32_t, 4> countData = { 0, 0, 0, 0 };
    lightsSSBO_->UpdateData(std::as_bytes(std::span(countData)), 0);
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
    std::array<uint32_t, 4> countData = { numLights, 0, 0, 0 };
    lightsSSBO_->UpdateData(std::as_bytes(std::span(countData)), 0);

    // Then update the array of LightData starting at offset sizeof(glm::vec4).
    if (!m_LightsData.empty()) {
        size_t dataSize = m_LightsData.size() * sizeof(LightData);
        lightsSSBO_->UpdateData(
            std::as_bytes(std::span(m_LightsData.data(), m_LightsData.size())),
            sizeof(glm::vec4)
        );
    }
}

void LightManager::BindLightsGPU() const
{
    if (lightsSSBO_) {
        lightsSSBO_->BindBase();
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
    // 1) Interpret light.position.xyz as the DIRECTION
    //    We'll normalize it. If it's near zero length, we need a fallback
    glm::vec3 dir = glm::normalize(glm::vec3(light.position));

    // 2) For "up" vector, pick something that isn't parallel to dir
    //    We'll default to (0,0,1). If dir is near that, fallback to (0,1,0).
    glm::vec3 up(0, 0, 1);
    if (fabs(glm::dot(dir, up)) > 0.99f) {
        up = glm::vec3(0, 1, 0);
    }

    // 3) Book approach: the "camera" is at the origin (0,0,0),
    //    looking TOWARD 'dir'
    //    So we do lookAt(0, dir, up).
    //    That means in light space, the light "eye" is at 0, and forward is +Z
    //    If you want forward = -Z, you can do lookAt(0, -dir, up).
    return glm::lookAt(glm::vec3(0.0f), dir, up);
}

glm::mat4 LightManager::ComputeDirectionalLightProj(const LightData& light) const
{
    // 1) Build the view matrix
    glm::mat4 lightView = ComputeDirectionalLightView(light);

    // 2) Transform the bounding box corners
    glm::vec3 corners[8] = {
        { m_BBox.min_.x, m_BBox.min_.y, m_BBox.min_.z },
        { m_BBox.min_.x, m_BBox.min_.y, m_BBox.max_.z },
        { m_BBox.min_.x, m_BBox.max_.y, m_BBox.min_.z },
        { m_BBox.min_.x, m_BBox.max_.y, m_BBox.max_.z },
        { m_BBox.max_.x, m_BBox.min_.y, m_BBox.min_.z },
        { m_BBox.max_.x, m_BBox.min_.y, m_BBox.max_.z },
        { m_BBox.max_.x, m_BBox.max_.y, m_BBox.min_.z },
        { m_BBox.max_.x, m_BBox.max_.y, m_BBox.max_.z },
    };

    glm::vec3 minLS(1e9f), maxLS(-1e9f);
    for (int i = 0; i < 8; i++)
    {
        glm::vec4 tmp = lightView * glm::vec4(corners[i], 1.0f);
        glm::vec3 v = glm::vec3(tmp);
        minLS = glm::min(minLS, v);
        maxLS = glm::max(maxLS, v);
    }

    // 3) The "book approach" for z => near/far = -maxLS.z, -minLS.z
    //    This flips the Z axis so we get a typical -Z forward camera orientation
    float left = minLS.x;
    float right = maxLS.x;
    float bottom = minLS.y;
    float top = maxLS.y;
    float zNear = -maxLS.z;
    float zFar = -minLS.z;

    // 4) Build ortho
    glm::mat4 lightProj = glm::ortho(left, right, bottom, top, zNear, zFar);
    return lightProj;
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