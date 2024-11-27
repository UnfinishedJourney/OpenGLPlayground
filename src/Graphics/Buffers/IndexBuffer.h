#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include "BufferDeleter.h"

class IndexBuffer {
public:
    IndexBuffer(std::span<const GLuint> data, GLenum usage = GL_STATIC_DRAW);
    ~IndexBuffer() = default;

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    void UpdateData(std::span<const GLuint> data, GLintptr offset = 0);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLsizei GetCount() const { return m_Count; }
    [[nodiscard]] size_t GetSize() const { return m_Size; }

private:
    GLuint m_RendererID = 0;
    GLsizei m_Count = 0;
    GLenum m_Usage = GL_STATIC_DRAW;
    size_t m_Size = 0;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};