#pragma once

#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include <memory>

class MeshBuffer {
public:
    MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

    void Bind() const;
    void Unbind() const;

    [[nodiscard]] size_t GetVertexCount() const { return m_VertexCount; }
    [[nodiscard]] size_t GetIndexCount() const { return m_IndexCount; }
    [[nodiscard]] const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VAO; }
    [[nodiscard]] const VertexBufferLayout& GetLayout() const { return m_Layout; }

private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VB;
    std::shared_ptr<IndexBuffer> m_IB;
    size_t m_VertexCount = 0;
    size_t m_IndexCount = 0;

    VertexBufferLayout m_Layout;

    void CreateBuffers(const Mesh& mesh, const MeshLayout& layout);
};