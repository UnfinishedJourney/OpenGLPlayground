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

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLsizei GetCount() const { return m_Count; }

private:
    GLuint m_RendererID = 0;
    GLsizei m_Count = 0;
};