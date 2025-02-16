#include "RenderObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// --- BaseRenderObject ---
bool BaseRenderObject::SetLOD(size_t lod) {
    if (lod == currentLOD_)
        return false;
    size_t maxLOD = (mesh_->lods_.empty()) ? 0 : (mesh_->lods_.size() - 1);
    if (lod > maxLOD) {
        lod = maxLOD;
    }
    currentLOD_ = lod;
    return true;
}

float BaseRenderObject::GetBoundingSphereRadius() const {
    return mesh_->boundingSphereRadius_;
}

glm::vec3 BaseRenderObject::GetCenter() const {
    return mesh_->localCenter_;
}

float BaseRenderObject::ComputeDistanceTo(const glm::vec3& pos) const {
    float dist = glm::distance(pos, GetCenter()) - GetBoundingSphereRadius();
    return (dist > 0.0f) ? dist : 0.0f;
}

// --- RenderObject ---
RenderObject::RenderObject(std::shared_ptr<graphics::Mesh> mesh,
    MeshLayout meshLayout,
    int materialID,
    std::string shaderName,
    std::shared_ptr<Transform> transform)
    : BaseRenderObject(std::move(mesh), std::move(meshLayout), materialID, std::move(shaderName))
    , transform_(std::move(transform))
{
}

bool RenderObject::SetLOD(size_t lod) {
    bool changed = BaseRenderObject::SetLOD(lod);
    // Optionally update GPU data if needed.
    return changed;
}

float RenderObject::GetBoundingSphereRadius() const {
    float baseRadius = BaseRenderObject::GetBoundingSphereRadius();
    glm::mat4 model = transform_->GetModelMatrix();
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(model[0]));
    scale.y = glm::length(glm::vec3(model[1]));
    scale.z = glm::length(glm::vec3(model[2]));
    float maxScale = std::max({ scale.x, scale.y, scale.z });
    return baseRadius * maxScale;
}

glm::vec3 RenderObject::GetCenter() const {
    return mesh_->localCenter_;
}

glm::vec3 RenderObject::GetWorldCenter() const {
    glm::vec3 local = GetCenter();
    glm::vec4 world = transform_->GetModelMatrix() * glm::vec4(local, 1.0f);
    return glm::vec3(world);
}

float RenderObject::ComputeDistanceTo(const glm::vec3& pos) const {
    return glm::distance(pos, GetWorldCenter()) - GetBoundingSphereRadius();
}

// --- StaticRenderObject ---
StaticRenderObject::StaticRenderObject(std::shared_ptr<graphics::Mesh> mesh,
    MeshLayout meshLayout,
    int materialID,
    std::string shaderName)
    : BaseRenderObject(std::move(mesh), std::move(meshLayout), materialID, std::move(shaderName))
{
}

glm::vec3 StaticRenderObject::GetWorldCenter() const {
    return mesh_->localCenter_;
}