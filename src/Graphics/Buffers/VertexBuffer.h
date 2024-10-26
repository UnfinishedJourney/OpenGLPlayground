#pragma once

#include <glad/glad.h>
#include <span>

class VertexBuffer {
public:
    VertexBuffer(std::span<const std::byte> data, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;
    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }

private:
    GLuint m_RendererID = 0;
    GLenum m_Usage = GL_STATIC_DRAW;
};