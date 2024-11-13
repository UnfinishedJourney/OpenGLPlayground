#pragma once

#include <glad/glad.h>
#include <span>
#include "Utilities/Logger.h"

class ShaderStorageBuffer {
public:
    ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);
    ~ShaderStorageBuffer();

    // Delete copy constructor and assignment operator
    ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
    ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

    // Allow move semantics
    ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
    ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0);

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }

private:
    GLuint m_RendererID = 0;
    GLuint m_BindingPoint = 0;
    GLsizeiptr m_Size = 0;
    GLenum m_Usage = GL_DYNAMIC_DRAW;
};