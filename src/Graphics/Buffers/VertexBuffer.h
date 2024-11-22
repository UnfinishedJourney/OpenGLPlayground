#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include <cstddef>
#include "BufferDeleter.h"

class VertexBuffer {
public:
    VertexBuffer(std::span<const std::byte> data, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer() = default;

    VertexBuffer(VertexBuffer&&) noexcept = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }

private:
    GLuint m_RendererID = 0;
    GLenum m_Usage = GL_STATIC_DRAW;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};