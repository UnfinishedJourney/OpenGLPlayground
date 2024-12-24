#include "RenderObject.h"
//#include <glm/gtx/matrix_decompose.hpp> // only if needed, but here we do not decompose

RenderObject::RenderObject(const std::shared_ptr<Mesh>& mesh,
    const MeshLayout& meshLayout,
    const std::string& materialName,
    const std::string& shaderName,
    const std::shared_ptr<Transform>& transform)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_Mesh(mesh),
    m_MeshLayout(meshLayout),
    m_Transform(transform),
    m_IsStatic(true),
    m_CurrentLOD(0)
{ }

const std::string& RenderObject::GetShaderName() const {
    return m_ShaderName;
}

const std::string& RenderObject::GetMaterialName() const {
    return m_MaterialName;
}

const std::shared_ptr<Mesh>& RenderObject::GetMesh() const {
    return m_Mesh;
}

const MeshLayout& RenderObject::GetMeshLayout() const {
    return m_MeshLayout;
}

const std::shared_ptr<Transform>& RenderObject::GetTransform() const {
    return m_Transform;
}

void RenderObject::Update(float /*deltaTime*/) {
    // For static geometry, do nothing.
    // If dynamic, you might manipulate the transform here.
}

glm::vec3 RenderObject::GetWorldCenter() const {
    glm::vec3 localCenter = m_Mesh->localCenter;
    glm::vec4 worldCenter4 = m_Transform->GetModelMatrix() * glm::vec4(localCenter, 1.0f);
    return glm::vec3(worldCenter4);
}

float RenderObject::GetBoundingSphereRadius() const {
    // Use max scale or approximate if non-uniform
    float maxScale = 1.0f;
    // If you keep track of scale in Transform or do partial decomposition, do it here
    // If you only store the final matrix, you might parse out scale from the matrix or just assume 1
    // For now, assume uniform scale or just do the approach below:
    // e.g. if we ignore scale, it's just the raw bounding sphere
    return m_Mesh->boundingSphereRadius * maxScale;
}

/**
 * The bounding-box approach:
 *  1) transform the local minBounds and maxBounds by the model matrix
 *  2) if pos is outside that box, compute distance to the box
 *  3) if inside, return 0
 */
float RenderObject::ComputeDistanceTo(const glm::vec3& pos) const {
    // Transform corners
    const glm::mat4& model = m_Transform->GetModelMatrix();
    glm::vec3 worldMin = glm::vec3(model * glm::vec4(m_Mesh->minBounds, 1.0f));
    glm::vec3 worldMax = glm::vec3(model * glm::vec4(m_Mesh->maxBounds, 1.0f));

    float distanceSq = 0.0f;
    for (int i = 0; i < 3; ++i) {
        float p = pos[i];
        if (p < worldMin[i]) {
            distanceSq += (worldMin[i] - p) * (worldMin[i] - p);
        }
        else if (p > worldMax[i]) {
            distanceSq += (p - worldMax[i]) * (p - worldMax[i]);
        }
    }
    return (distanceSq > 0.0f) ? std::sqrt(distanceSq) : 0.0f;
}

size_t RenderObject::GetCurrentLOD() const {
    return m_CurrentLOD;
}

bool RenderObject::SetLOD(size_t lodLevel) {
    if (lodLevel == m_CurrentLOD) {
        return false;
    }
    size_t maxLOD = m_Mesh->lods.size() - 1;
    if (lodLevel > maxLOD) {
        lodLevel = maxLOD;
    }
    m_CurrentLOD = lodLevel;
    return true;
}