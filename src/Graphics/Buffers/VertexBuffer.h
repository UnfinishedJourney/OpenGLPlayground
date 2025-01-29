#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include <cstddef>
#include "BufferDeleter.h"

/**
 * @brief Encapsulates an OpenGL Vertex Buffer Object (VBO).
 *
 * Stores raw vertex data (in bytes). Typically bound to GL_ARRAY_BUFFER
 * and attached to a VAO via VertexArray::AddBuffer().
 */
class VertexBuffer {
public:
    /**
     * @param data  A span of raw bytes. Could contain positions, normals, etc.
     * @param usage Typical usage pattern (e.g., GL_STATIC_DRAW, GL_DYNAMIC_DRAW).
     */
    VertexBuffer(std::span<const std::byte> data, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer() = default;

    VertexBuffer(VertexBuffer&&) noexcept = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    /**
     * @brief Binds this VBO to GL_ARRAY_BUFFER.
     */
    void Bind() const;

    /**
     * @brief Unbinds the VBO (binds 0 to GL_ARRAY_BUFFER).
     */
    void Unbind() const;

    /**
     * @brief Updates a subregion of the buffer with new data.
     * @throws std::runtime_error if offset + size exceeds buffer capacity.
     */
    void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

    /**
     * @return The size of the buffer (in bytes).
     */
    [[nodiscard]] size_t GetSize() const { return m_Size; }

    /**
     * @return The OpenGL buffer ID (handle).
     */
    [[nodiscard]] GLuint GetRendererID() const { return m_RendererIDPtr ? *m_RendererIDPtr : 0; }

private:
    size_t m_Size = 0;
    GLenum m_Usage = GL_STATIC_DRAW;

    // RAII-managed buffer ID
    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};