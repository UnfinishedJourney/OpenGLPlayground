#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <variant>
#include <span>
#include "Utilities/Logger.h"

class UniformBuffer {
public:
    UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);
    ~UniformBuffer();

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    // Updated SetData method
    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

    // New methods for buffer mapping
    void* MapBuffer(GLenum access) const;
    void UnmapBuffer() const;

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }
    [[nodiscard]] GLsizeiptr GetSize() const { return m_Size; }

private:
    GLuint m_RendererID = 0;
    GLuint m_BindingPoint = 0;
    GLenum m_Usage = GL_DYNAMIC_DRAW;
    GLsizeiptr m_Size = 0;
};