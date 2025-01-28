#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include <cstddef>
#include "BufferDeleter.h"

/**
 * @brief Manages an OpenGL buffer for indirect draw commands (GL_DRAW_INDIRECT_BUFFER).
 */
class IndirectBuffer {
public:
    /**
     * @param data  The raw byte data to store in the buffer (e.g., DrawArraysIndirectCommand).
     * @param usage Typical usage pattern (GL_DYNAMIC_DRAW, etc.).
     */
    IndirectBuffer(std::span<const std::byte> data, GLenum usage = GL_DYNAMIC_DRAW);
    ~IndirectBuffer() = default;

    IndirectBuffer(const IndirectBuffer&) = delete;
    IndirectBuffer& operator=(const IndirectBuffer&) = delete;

    IndirectBuffer(IndirectBuffer&&) noexcept = default;
    IndirectBuffer& operator=(IndirectBuffer&&) noexcept = default;

    /**
     * @brief Binds this buffer as GL_DRAW_INDIRECT_BUFFER.
     */
    void Bind() const;

    /**
     * @brief Unbinds the draw indirect buffer.
     */
    void Unbind() const;

    /**
     * @brief Updates a subregion of the buffer with new data.
     */
    void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererIDPtr ? *m_RendererIDPtr : 0; }

private:
    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};