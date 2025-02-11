#include "RenderObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// ------------------------------------------------------------------
// BaseRenderObject
// ------------------------------------------------------------------
bool BaseRenderObject::SetLOD(size_t lod)
{
    if (lod == m_CurrentLOD) {
        return false; // no change
    }
    size_t maxLOD = (m_Mesh->lods_.empty()) ? 0 : (m_Mesh->lods_.size() - 1);
    if (lod > maxLOD) {
        lod = maxLOD;
    }
    m_CurrentLOD = lod;
    return true;
}

float BaseRenderObject::GetBoundingSphereRadius() const
{
    return m_Mesh->boundingSphereRadius_;
}

glm::vec3 BaseRenderObject::GetCenter() const
{
    return m_Mesh->localCenter_;
}

float BaseRenderObject::ComputeDistanceTo(const glm::vec3& pos) const
{
    float dist = glm::distance(pos, GetCenter()) - GetBoundingSphereRadius();
    return (dist > 0.0f) ? dist : 0.0f;
}

// ------------------------------------------------------------------
// RenderObject (dynamic object with a Transform)
// ------------------------------------------------------------------
RenderObject::RenderObject(std::shared_ptr<Graphics::Mesh> mesh,
    MeshLayout meshLayout,
    int materialID,
    std::string shaderName,
    std::shared_ptr<Transform> transform)
    : BaseRenderObject(std::move(mesh), std::move(meshLayout), materialID, std::move(shaderName))
    , m_Transform(std::move(transform))
{
}

bool RenderObject::SetLOD(size_t lod)
{
    bool changed = BaseRenderObject::SetLOD(lod);
    // Possibly update GPU data or something if needed
    return changed;
}

float RenderObject::GetBoundingSphereRadius() const
{
    // Scale bounding sphere by the largest axis scale
    float baseRadius = BaseRenderObject::GetBoundingSphereRadius();
    glm::mat4 model = m_Transform->GetModelMatrix();
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(model[0]));
    scale.y = glm::length(glm::vec3(model[1]));
    scale.z = glm::length(glm::vec3(model[2]));
    float maxScale = std::max({ scale.x, scale.y, scale.z });
    return baseRadius * maxScale;
}

glm::vec3 RenderObject::GetCenter() const
{
    return m_Mesh->localCenter_;
}

glm::vec3 RenderObject::GetWorldCenter() const
{
    glm::vec3 local = GetCenter();
    glm::vec4 world = m_Transform->GetModelMatrix() * glm::vec4(local, 1.0f);
    return glm::vec3(world);
}

float RenderObject::ComputeDistanceTo(const glm::vec3& pos) const
{
    // approximate bounding box approach or bounding sphere approach
    return glm::distance(pos, GetWorldCenter()) - GetBoundingSphereRadius();
}

// ------------------------------------------------------------------
// StaticRenderObject
// ------------------------------------------------------------------
StaticRenderObject::StaticRenderObject(std::shared_ptr<Graphics::Mesh> mesh,
    MeshLayout meshLayout,
    int materialID,
    std::string shaderName)
    : BaseRenderObject(std::move(mesh), std::move(meshLayout), materialID, std::move(shaderName))
{
}

glm::vec3 StaticRenderObject::GetWorldCenter() const
{
    // local = world for static
    return m_Mesh->localCenter_;
}