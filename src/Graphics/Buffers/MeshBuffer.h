#pragma once

#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include <memory>

//maybe need to remove smart pointers
class MeshBuffer {
public:
    MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

    void Bind() const;
    void Unbind() const;
    void Render() const;

    GLuint GetIndexCount() const { return m_IndexCount; }
    GLuint GetVertexCount() const { return m_VertexCount; }
    bool HasIndices() const { return m_BHasIndices; }

private:
    bool m_BHasIndices = false;
    MeshLayout m_MeshLayout;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer> m_IBO;
    GLuint m_VertexCount = 0;
    GLuint m_IndexCount = 0;
};