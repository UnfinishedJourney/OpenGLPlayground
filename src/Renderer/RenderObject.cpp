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