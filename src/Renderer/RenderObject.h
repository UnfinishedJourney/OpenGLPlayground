#pragma once

#include <memory>
#include <string>

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

    virtual ~RenderObject() = default;

    // Accessors
    const std::string& GetShaderName() const;
    const std::string& GetMaterialName() const;
    const std::shared_ptr<Mesh>& GetMesh() const;
    const MeshLayout& GetMeshLayout() const;
    const std::shared_ptr<Transform>& GetTransform() const;

    virtual void Update(float deltaTime);

protected:
    std::string m_ShaderName;
    std::string m_MaterialName;
    std::shared_ptr<Mesh> m_Mesh;
    MeshLayout m_MeshLayout;
    std::shared_ptr<Transform> m_Transform;
};