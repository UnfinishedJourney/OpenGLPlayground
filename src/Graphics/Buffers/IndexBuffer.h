#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include "BufferDeleter.h"

/**
 * @brief Manages an OpenGL Index Buffer Object (IBO) for element drawing.
 */
class IndexBuffer {
public:
    /**
     * @param data  The index data (GLuints).
     * @param usage Typical usage pattern (GL_STATIC_DRAW, etc.).
     */
    IndexBuffer(std::span<const GLuint> data, GLenum usage = GL_STATIC_DRAW);
    ~IndexBuffer() = default;

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    /**
     * @brief Binds this IBO to GL_ELEMENT_ARRAY_BUFFER.
     */
    void Bind() const;

    /**
     * @brief Unbinds the IBO from GL_ELEMENT_ARRAY_BUFFER.
     */
    void Unbind() const;

    /**
     * @brief Updates a subregion of the buffer with new index data.
     * @throws std::runtime_error if the new data exceeds buffer size.
     */
    void UpdateData(std::span<const GLuint> data, GLintptr offset = 0);

    [[nodiscard]] GLuint  GetRendererID() const { return *m_RendererIDPtr; }
    [[nodiscard]] GLsizei GetCount()      const { return m_Count; }
    [[nodiscard]] size_t  GetSize()       const { return m_Size; }

private:
    GLsizei m_Count = 0;
    GLenum  m_Usage = GL_STATIC_DRAW;
    size_t  m_Size = 0;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};