#pragma once

#include <glad/glad.h>

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size, GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW);

    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;
    [[nodiscard]]  unsigned int GetRendererID() const { return m_RendererID; }
    [[nodiscard]]  GLenum GetTarget() const { return m_Target; }

private:
    unsigned int m_RendererID;
    GLenum m_Target;
    GLenum m_Usage;
};