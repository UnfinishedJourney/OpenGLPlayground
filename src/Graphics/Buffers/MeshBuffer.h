#pragma once

#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include <memory>
#include <vector>
#include <span>

class MeshBuffer {
public:
    MeshBuffer(const Mesh& mesh, const MeshLayout& layout);
    ~MeshBuffer() = default;

    MeshBuffer(const MeshBuffer&) = delete;
    MeshBuffer& operator=(const MeshBuffer&) = delete;

    MeshBuffer(MeshBuffer&& other) noexcept = default;
    MeshBuffer& operator=(MeshBuffer&& other) noexcept = default;

    void Bind() const;
    void Unbind() const;

    [[nodiscard]] size_t GetVertexCount() const { return m_VertexCount; }
    [[nodiscard]] size_t GetIndexCount() const { return m_IndexCount; }
    [[nodiscard]] const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VAO; }
    [[nodiscard]] const VertexBufferLayout& GetLayout() const { return m_Layout; }

    // New methods to access raw data
    [[nodiscard]] const std::vector<std::byte>& GetVertexData() const { return m_VertexData; }
    [[nodiscard]] const std::vector<GLuint>& GetIndexData() const { return m_IndexData; }

private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VB;
    std::shared_ptr<IndexBuffer> m_IB;
    size_t m_VertexCount = 0;
    size_t m_IndexCount = 0;

    VertexBufferLayout m_Layout;

    // Store raw data for batching
    std::vector<std::byte> m_VertexData;
    std::vector<GLuint> m_IndexData;

    //void InterleaveVertexData(const Mesh& mesh, const MeshLayout& layout);
};