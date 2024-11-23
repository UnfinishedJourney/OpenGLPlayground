#pragma once

#include <glad/glad.h>
#include <memory>
#include <span>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include "BufferDeleter.h"

class ShaderStorageBuffer {
public:
    ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);
    ~ShaderStorageBuffer() = default;

    ShaderStorageBuffer(ShaderStorageBuffer&&) noexcept = default;
    ShaderStorageBuffer& operator=(ShaderStorageBuffer&&) noexcept = default;

    ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
    ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0);
    void BindBase() const; // Ensure the buffer is bound to the binding point

    GLsizeiptr GetSize() const { return m_Size; }

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }

private:
    GLuint m_BindingPoint = 0;
    GLsizeiptr m_Size = 0;
    GLenum m_Usage = GL_DYNAMIC_DRAW;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};