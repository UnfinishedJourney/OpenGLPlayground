#pragma once

#include <glad/glad.h>
#include <memory>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include "BufferDeleter.h"

/**
 * @brief Manages an OpenGL Shader Storage Buffer (GL_SHADER_STORAGE_BUFFER).
 *
 * Allows binding at a specified binding point for use in shader programs.
 */
class ShaderStorageBuffer {
public:
    /**
     * @param bindingPoint The binding point index to bind this SSBO.
     * @param size         The total size (in bytes) of the buffer.
     * @param usage        Typical usage (GL_DYNAMIC_DRAW, etc.).
     */
    ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);
    ~ShaderStorageBuffer() = default;

    ShaderStorageBuffer(ShaderStorageBuffer&&) noexcept = default;
    ShaderStorageBuffer& operator=(ShaderStorageBuffer&&) noexcept = default;

    ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
    ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

    /**
     * @brief Binds this SSBO to GL_SHADER_STORAGE_BUFFER (without binding base).
     */
    void Bind() const;

    /**
     * @brief Unbinds the SSBO from GL_SHADER_STORAGE_BUFFER.
     */
    void Unbind() const;

    /**
     * @brief Updates a region of the buffer with new data (subdata).
     * @throws std::runtime_error if size + offset exceed the buffer capacity.
     */
    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0);

    /**
     * @brief Binds the SSBO to its specified binding point.
     */
    void BindBase() const;

    [[nodiscard]] GLsizeiptr GetSize()         const { return m_Size; }
    [[nodiscard]] GLuint     GetRendererID()   const { return *m_RendererIDPtr; }
    [[nodiscard]] GLuint     GetBindingPoint() const { return m_BindingPoint; }

private:
    GLuint      m_BindingPoint = 0;
    GLsizeiptr  m_Size = 0;
    GLenum      m_Usage = GL_DYNAMIC_DRAW;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};