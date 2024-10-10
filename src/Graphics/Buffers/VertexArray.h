#pragma once

#include <glad/glad.h>
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"

class VertexArray
{
public:
    VertexArray();

    VertexArray(unsigned int rendererID);

    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&&) noexcept = default;
    VertexArray& operator=(VertexArray&&) noexcept = default;

    void AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout) const;
    void AddInstancedBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout, unsigned int divisor) const;

    // ?????? ???????? ? ??????? VAO
    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_RendererID;
};