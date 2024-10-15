#pragma once

#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"

#include <glad/glad.h>

#include <vector>
#include <stdexcept>

class VertexArray {
public:
    VertexArray();
    explicit VertexArray(unsigned int rendererID);
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout) const;
    void AddInstancedBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout, unsigned int divisor) const;
    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_RendererID;
};