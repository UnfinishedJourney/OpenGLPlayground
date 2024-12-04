#include "Renderer/RenderObject.h"

RenderObject::RenderObject(const std::shared_ptr<Mesh>& mesh,
    const MeshLayout& meshLayout,
    const std::string& materialName,
    const std::string& shaderName,
    const std::shared_ptr<Transform>& transform)
    : m_Mesh(mesh),
    m_MeshLayout(meshLayout),
    m_MaterialName(materialName),
    m_ShaderName(shaderName),
    m_Transform(transform)
{
    // Initialization if needed
}

const std::string& RenderObject::GetShaderName() const { return m_ShaderName; }
const std::string& RenderObject::GetMaterialName() const { return m_MaterialName; }
const std::shared_ptr<Mesh>& RenderObject::GetMesh() const { return m_Mesh; }
const MeshLayout& RenderObject::GetMeshLayout() const { return m_MeshLayout; }
const std::shared_ptr<Transform>& RenderObject::GetTransform() const { return m_Transform; }

void RenderObject::Update(float deltaTime)
{
    // Implement per-frame updates if necessary
}

float RenderObject::ComputeDistanceTo(const glm::vec3& pos) const {
    // Transform min and max bounds to world space
    glm::vec3 worldMin = glm::vec3(GetTransform()->GetModelMatrix() * glm::vec4(m_Mesh->minBounds, 1.0f));
    glm::vec3 worldMax = glm::vec3(GetTransform()->GetModelMatrix() * glm::vec4(m_Mesh->maxBounds, 1.0f));

    // Compute squared distance from camera to AABB
    float distanceSq = 0.0f;
    for (int i = 0; i < 3; ++i) {
        float v = pos[i];
        if (v < worldMin[i]) distanceSq += (worldMin[i] - v) * (worldMin[i] - v);
        if (v > worldMax[i]) distanceSq += (v - worldMax[i]) * (v - worldMax[i]);
    }
    return sqrt(distanceSq);
}

glm::vec3 RenderObject::GetWorldCenter() const {
    return glm::vec3(GetTransform()->GetModelMatrix() * glm::vec4(m_Mesh->localCenter, 1.0f));
}

float RenderObject::GetBoundingSphereRadius() const {
    // Assuming uniform scaling, adjust the radius by the scale factor
    // If non-uniform scaling is used, consider computing an oriented bounding box or an ellipsoid
    float scale = glm::length(GetTransform()->GetScale());
    return m_Mesh->boundingSphereRadius * scale;
}