#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Scene/Transform.h"

class RenderObject {
public:
    RenderObject(const std::shared_ptr<Mesh>& mesh,
        const MeshLayout& meshLayout,
        const std::string& materialName,
        const std::string& shaderName,
        const std::shared_ptr<Transform>& transform);

    ~RenderObject() = default;

    const std::string& GetShaderName() const;
    const std::string& GetMaterialName() const;
    const std::shared_ptr<Mesh>& GetMesh() const;
    const MeshLayout& GetMeshLayout() const;
    const std::shared_ptr<Transform>& GetTransform() const;

    void Update(float deltaTime);

    glm::vec3 GetWorldCenter() const;
    float GetBoundingSphereRadius() const;
    float ComputeDistanceTo(const glm::vec3& pos) const;

    bool IsStatic() const { return m_IsStatic; }

    // LOD
    size_t GetCurrentLOD() const { return m_CurrentLOD; }
    bool SetLOD(size_t lodLevel) {
        if (m_CurrentLOD != lodLevel) {
            m_CurrentLOD = lodLevel;
            return true;
        }
        return false;
    }

private:
    std::string m_ShaderName;
    std::string m_MaterialName;
    std::shared_ptr<Mesh> m_Mesh;
    MeshLayout m_MeshLayout;
    std::shared_ptr<Transform> m_Transform;
    bool m_IsStatic = true;
    size_t m_CurrentLOD = 0;
};