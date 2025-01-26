#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Scene/Transform.h"
#include <algorithm>

class BaseRenderObject {
public:
    virtual ~BaseRenderObject() = default;

    // Accessor methods
    const std::string& GetShaderName() const { return m_ShaderName; }
    int GetMaterialID() const { return m_MaterialID; }
    const std::shared_ptr<Mesh>& GetMesh() const { return m_Mesh; }
    const MeshLayout& GetMeshLayout() const { return m_MeshLayout; }

    // Bounding information
    glm::vec3 GetCenter() const { return m_Mesh->localCenter; }
    virtual float GetBoundingSphereRadius() const { return m_Mesh->boundingSphereRadius; }

    // Distance computation
    virtual float ComputeDistanceTo(const glm::vec3& pos) const {
        float distance = glm::distance(pos, GetCenter()) - GetBoundingSphereRadius();
        return distance > 0.0f ? distance : 0.0f;
    }

    // LOD management
    size_t GetCurrentLOD() const { return m_CurrentLOD; }
    virtual bool SetLOD(size_t lodLevel) {
        if (lodLevel == m_CurrentLOD) {
            return false;
        }
        size_t maxLOD = (m_Mesh->lods.empty()) ? 0 : (m_Mesh->lods.size() - 1);
        if (lodLevel > maxLOD) {
            lodLevel = maxLOD;
        }
        m_CurrentLOD = lodLevel;
        return true;
    }

protected:
    // Constructor is protected to enforce inheritance
    BaseRenderObject(const std::shared_ptr<Mesh>& mesh,
        const MeshLayout& meshLayout,
        int materialID,
        const std::string& shaderName)
        : m_ShaderName(shaderName),
        m_MaterialID(materialID),
        m_Mesh(mesh),
        m_MeshLayout(meshLayout),
        m_CurrentLOD(0)
    { }

    std::string m_ShaderName;
    int m_MaterialID;
    std::shared_ptr<Mesh> m_Mesh;
    MeshLayout m_MeshLayout;
    size_t m_CurrentLOD = 0;
};

// ========================
// RenderObject.h
// ========================

class RenderObject : public BaseRenderObject {
public:
    RenderObject(const std::shared_ptr<Mesh>& mesh,
        const MeshLayout& meshLayout,
        int materialID,
        const std::string& shaderName,
        const std::shared_ptr<Transform>& transform)
        : BaseRenderObject(mesh, meshLayout, materialID, shaderName),
        m_Transform(transform),
        m_IsStatic(true)
    { }

    ~RenderObject() override = default;

    // Accessor for transform
    const std::shared_ptr<Transform>& GetTransform() const { return m_Transform; }

    // Update method for dynamic objects
    void Update(float deltaTime) {
        if (!m_IsStatic) {
            // Implement dynamic updates here (e.g., animations, physics)
        }
    }

    // World-space center computation
    glm::vec3 GetWorldCenter() const {
        glm::vec3 localCenter = GetCenter();
        glm::vec4 worldCenter4 = m_Transform->GetModelMatrix() * glm::vec4(localCenter, 1.0f);
        return glm::vec3(worldCenter4);
    }

    // Override bounding sphere to account for scaling
    float GetBoundingSphereRadius() const override {
        glm::vec3 scale;
        const glm::mat4& model = m_Transform->GetModelMatrix();
        scale.x = glm::length(glm::vec3(model[0]));
        scale.y = glm::length(glm::vec3(model[1]));
        scale.z = glm::length(glm::vec3(model[2]));
        float maxScale = std::max({ scale.x, scale.y, scale.z });
        return m_Mesh->boundingSphereRadius * maxScale;
    }

    // Override distance computation to use world-space bounds
    float ComputeDistanceTo(const glm::vec3& pos) const override {
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

    // Static flag accessor
    bool IsStatic() const { return m_IsStatic; }

private:
    std::shared_ptr<Transform> m_Transform;
    bool m_IsStatic = true;
};