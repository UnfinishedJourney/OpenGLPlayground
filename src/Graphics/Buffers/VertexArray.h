#pragma once

#include <glad/glad.h>
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout);
    void Bind() const;
    void Unbind() const;

    [[nodiscard]] GLuint GetRendererID() const
    {
        return m_RendererID;
    }

private:
    GLuint m_RendererID = 0;
};