#pragma once

#include <glad/glad.h>
#include <memory>
#include <span>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include "BufferDeleter.h"

/**
 * @brief Manages an OpenGL Uniform Buffer Object (UBO).
 *
 * Provides base binding and subdata updates, as well as optional
 * map/unmap functions for direct data access.
 */
class UniformBuffer {
public:
    /**
     * @param size          The size of the buffer in bytes.
     * @param bindingPoint  The binding point index to bind this UBO.
     * @param usage         Typical usage pattern (e.g., GL_DYNAMIC_DRAW).
     */
    UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);
    ~UniformBuffer() = default;

    UniformBuffer(UniformBuffer&&) noexcept = default;
    UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    /**
     * @brief Bind this UBO to its binding point (GL_UNIFORM_BUFFER).
     */
    void Bind() const;

    /**
     * @brief Unbind any UBO from the specified binding point.
     */
    void Unbind() const;

    /**
     * @brief Update a subrange of the buffer with new data.
     * @note  Assumes 16-byte alignment for offset/size if you're using std140.
     * @throws std::runtime_error on alignment error or out-of-range update.
     */
    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

    /**
     * @brief Map the entire buffer or a range (when combined with glMapBufferRange) for direct access.
     * @param access e.g., GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE
     * @return Pointer to the mapped memory (or nullptr on failure).
     */
    void* MapBuffer(GLenum access) const;

    /**
     * @brief Unmap the buffer after a map operation.
     */
    void UnmapBuffer() const;

    [[nodiscard]] GLuint     GetRendererID()   const { return *m_RendererIDPtr; }
    [[nodiscard]] GLuint     GetBindingPoint() const { return m_BindingPoint; }
    [[nodiscard]] GLsizeiptr GetSize()         const { return m_Size; }

private:
    GLuint     m_BindingPoint = 0;
    GLenum     m_Usage = GL_DYNAMIC_DRAW;
    GLsizeiptr m_Size = 0;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};