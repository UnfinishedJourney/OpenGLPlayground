#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "BufferDeleter.h"

class VertexArray {
public:
    VertexArray();
    ~VertexArray() = default;

    VertexArray(VertexArray&&) noexcept = default;
    VertexArray& operator=(VertexArray&&) noexcept = default;

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, GLuint bindingIndex = 0);
    void SetIndexBuffer(const IndexBuffer& indexBuffer);

    void Bind() const;
    void Unbind() const;

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }

private:
    std::unique_ptr<GLuint, VertexArrayDeleter> m_RendererIDPtr;

    std::vector<GLuint> m_BindingIndices;
};