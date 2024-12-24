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

    // For static objects, might be empty:
    void Update(float deltaTime);

    // Bounding info
    glm::vec3 GetWorldCenter() const;
    float GetBoundingSphereRadius() const;

    /**
     * This method returns the distance from `pos` to this
     * RenderObject’s world-aligned bounding box.
     * If `pos` is inside the box, distance is 0.
     */
    float ComputeDistanceTo(const glm::vec3& pos) const;

    size_t GetCurrentLOD() const;
    bool   SetLOD(size_t lodLevel);

    bool IsStatic() const { return m_IsStatic; }

private:
    std::string m_ShaderName;
    std::string m_MaterialName;
    std::shared_ptr<Mesh> m_Mesh;
    MeshLayout  m_MeshLayout;
    std::shared_ptr<Transform> m_Transform;

    bool   m_IsStatic = true;
    size_t m_CurrentLOD = 0;
};