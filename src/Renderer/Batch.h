#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Renderer/RenderObject.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"

class Batch {
public:
    Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout);
    ~Batch();

    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void BuildBatch();
    void Render() const;

    // Accessors
    const std::string& GetShaderName() const { return m_ShaderName; }
    const std::string& GetMaterialName() const { return m_MaterialName; }
    const MeshLayout& GetMeshLayout() const { return m_MeshLayout; }
    const std::shared_ptr<Transform>& GetTransform() const { return m_RenderObjects[0]->GetTransform(); }

private:
    std::string m_ShaderName;
    std::string m_MaterialName;
    MeshLayout m_MeshLayout;
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;

    // Combined buffers for the batch
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer> m_IBO;
    GLsizei m_IndexCount = 0;
};