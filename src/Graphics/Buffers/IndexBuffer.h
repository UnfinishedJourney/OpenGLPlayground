#pragma once

#include <span>
#include <glad/glad.h>

class IndexBuffer {
public:
    IndexBuffer(std::span<const GLuint> data);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;
    [[nodiscard]] GLsizei GetCount() const;

private:
    GLuint m_RendererID = 0;
    GLsizei m_Count = 0;
};