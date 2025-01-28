#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "BufferDeleter.h"

/**
 * @brief Encapsulates an OpenGL Vertex Array Object (VAO).
 *
 * Manages binding of vertex buffers, element buffers (IBO),
 * and the vertex attribute configurations.
 */
class VertexArray {
public:
    VertexArray();
    ~VertexArray() = default;

    VertexArray(VertexArray&&) noexcept = default;
    VertexArray& operator=(VertexArray&&) noexcept = default;

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    /**
     * @brief Attach a vertex buffer to this VAO with a given layout.
     * @param vertexBuffer  The source VertexBuffer (already containing data).
     * @param layout        The layout describing how to interpret the data.
     * @param bindingIndex  The binding slot for this data in the VAO.
     */
    void AddBuffer(const VertexBuffer& vertexBuffer,
        const VertexBufferLayout& layout,
        GLuint bindingIndex = 0);

    /**
     * @brief Set the IndexBuffer (element buffer) for this VAO.
     */
    void SetIndexBuffer(const IndexBuffer& indexBuffer);

    /**
     * @brief Bind this VAO as the current vertex array.
     */
    void Bind() const;

    /**
     * @brief Unbind any vertex array (binds VAO=0).
     */
    void Unbind() const;

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }

private:
    std::unique_ptr<GLuint, VertexArrayDeleter> m_RendererIDPtr;
    std::vector<GLuint>                         m_BindingIndices; ///< Tracks used binding slots
};