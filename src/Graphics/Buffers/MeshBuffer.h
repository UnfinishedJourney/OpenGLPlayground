#pragma once

#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include <glm/glm.hpp>
#include <memory>

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

    [[nodiscard]] size_t GetVertexCount() const { return m_IndexCount; }
    [[nodiscard]] const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VAO; }

private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VB;
    std::shared_ptr<IndexBuffer> m_IB;
    size_t m_IndexCount = 0;
};