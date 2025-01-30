#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Scene/Transform.h"

/**
 * @brief Base class for something that can be rendered.
 *        It references a Mesh, a material ID, and a shader name.
 */
class BaseRenderObject {
public:
    BaseRenderObject(std::shared_ptr<Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName)
        : m_Mesh(std::move(mesh))
        , m_MeshLayout(std::move(meshLayout))
        , m_MaterialID(materialID)
        , m_ShaderName(std::move(shaderName))
    {}

    virtual ~BaseRenderObject() = default;

    // Accessors
    const std::shared_ptr<Mesh>& GetMesh()       const { return m_Mesh; }
    const MeshLayout& GetMeshLayout() const { return m_MeshLayout; }
    int                          GetMaterialID() const { return m_MaterialID; }
    const std::string& GetShaderName() const { return m_ShaderName; }

    // LOD
    size_t GetCurrentLOD() const { return m_CurrentLOD; }
    virtual bool  SetLOD(size_t lod);
    virtual float GetBoundingSphereRadius() const;
    virtual glm::vec3 GetCenter()     const;
    virtual glm::vec3 GetWorldCenter() const { return GetCenter(); }

    // Distance-based logic
    virtual float ComputeDistanceTo(const glm::vec3& pos) const;

protected:
    std::shared_ptr<Mesh> m_Mesh;
    MeshLayout            m_MeshLayout;
    int                   m_MaterialID;
    std::string           m_ShaderName;

    // LOD index
    size_t m_CurrentLOD = 0;
};

class RenderObject : public BaseRenderObject {
public:
    RenderObject(std::shared_ptr<Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName,
        std::shared_ptr<Transform> transform);

    ~RenderObject() override = default;

    const std::shared_ptr<Transform>& GetTransform() const { return m_Transform; }

    bool   SetLOD(size_t lod) override;
    float  GetBoundingSphereRadius() const override;
    glm::vec3 GetCenter()     const override;
    glm::vec3 GetWorldCenter() const override;

    float ComputeDistanceTo(const glm::vec3& pos) const override;

private:
    std::shared_ptr<Transform> m_Transform;
};

class StaticRenderObject : public BaseRenderObject {
public:
    StaticRenderObject(std::shared_ptr<Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName);

    ~StaticRenderObject() override = default;

    // For static objects, center & bounding sphere = local is world
    glm::vec3 GetWorldCenter() const override;
};