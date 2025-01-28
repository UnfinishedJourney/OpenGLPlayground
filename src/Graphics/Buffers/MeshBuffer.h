#pragma once

#include <memory>
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"

/**
 * @brief Combines a VAO, VBO, and optional IBO into one renderable mesh buffer.
 *
 * Binds vertex attributes according to the provided MeshLayout.
 */
class MeshBuffer {
public:
    /**
     * @param mesh   The geometry (positions, normals, indices, etc.).
     * @param layout The layout describing which attributes to include.
     */
    MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

    /**
     * @brief Binds the VAO (and IBO if present).
     */
    void Bind() const;

    /**
     * @brief Unbinds the VAO (and IBO if present).
     */
    void Unbind() const;

    /**
     * @brief Calls glDrawElements or glDrawArrays depending on presence of indices.
     */
    void Render() const;

    [[nodiscard]] GLuint GetIndexCount()  const { return m_IndexCount; }
    [[nodiscard]] GLuint GetVertexCount() const { return m_VertexCount; }
    [[nodiscard]] bool   HasIndices()     const { return m_BHasIndices; }

private:
    bool        m_BHasIndices = false;
    MeshLayout  m_MeshLayout;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer>  m_IBO;

    GLuint m_VertexCount = 0;
    GLuint m_IndexCount = 0;
};