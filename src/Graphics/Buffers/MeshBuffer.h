#pragma once

#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include <memory>

class MeshBuffer {
public:
    MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

    void Bind() const;
    void Unbind() const;
    void Render() const;

    GLuint GetIndexCount() const { return m_IndexCount; } // Added getter for index count

private:
    MeshLayout m_MeshLayout;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer> m_IBO;
    GLuint m_VertexCount;
    GLuint m_IndexCount;
};