#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Scene/Transform.h"

/**
 * Base class for renderable objects.
 */
class BaseRenderObject {
public:
    BaseRenderObject(std::shared_ptr<graphics::Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName)
        : mesh_(std::move(mesh))
        , meshLayout_(std::move(meshLayout))
        , materialID_(materialID)
        , shaderName_(std::move(shaderName))
    {}

    virtual ~BaseRenderObject() = default;

    const std::shared_ptr<graphics::Mesh>& GetMesh() const { return mesh_; }
    const MeshLayout& GetMeshLayout() const { return meshLayout_; }
    int GetMaterialID() const { return materialID_; }
    const std::string& GetShaderName() const { return shaderName_; }

    size_t GetCurrentLOD() const { return currentLOD_; }
    virtual bool SetLOD(size_t lod);
    virtual float GetBoundingSphereRadius() const;
    virtual glm::vec3 GetCenter() const;
    virtual glm::vec3 GetWorldCenter() const { return GetCenter(); }
    virtual float ComputeDistanceTo(const glm::vec3& pos) const;

    int GetVertexCount() const { return mesh_->positions_.size(); }
    int GetIndexCount() const { return mesh_->indices_.size(); }

protected:
    std::shared_ptr<graphics::Mesh> mesh_;
    MeshLayout meshLayout_;
    int materialID_;
    std::string shaderName_;
    size_t currentLOD_ = 0;
};

class RenderObject : public BaseRenderObject {
public:
    RenderObject(std::shared_ptr<graphics::Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName,
        std::shared_ptr<Transform> transform);

    ~RenderObject() override = default;

    const std::shared_ptr<Transform>& GetTransform() const { return transform_; }

    bool SetLOD(size_t lod) override;
    float GetBoundingSphereRadius() const override;
    glm::vec3 GetCenter() const override;
    glm::vec3 GetWorldCenter() const override;
    float ComputeDistanceTo(const glm::vec3& pos) const override;

private:
    std::shared_ptr<Transform> transform_;
};

class StaticRenderObject : public BaseRenderObject {
public:
    StaticRenderObject(std::shared_ptr<graphics::Mesh> mesh,
        MeshLayout meshLayout,
        int materialID,
        std::string shaderName);
    ~StaticRenderObject() override = default;

    glm::vec3 GetWorldCenter() const override;
};
