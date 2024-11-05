#pragma once

#include <span>
#include <glad/glad.h>

class IndexBuffer {
public:
    IndexBuffer(std::span<const GLuint> data, GLenum usage = GL_STATIC_DRAW);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    void UpdateData(std::span<const GLuint> data, GLintptr offset = 0);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLsizei GetCount() const { return m_Count; }

private:
    GLuint m_RendererID = 0;
    GLsizei m_Count = 0;
    GLenum m_Usage = GL_STATIC_DRAW;
};