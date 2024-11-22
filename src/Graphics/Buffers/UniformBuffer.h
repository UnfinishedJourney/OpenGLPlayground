#pragma once

#include <glad/glad.h>
#include <memory>
#include <span>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include "BufferDeleter.h"

class UniformBuffer {
public:
    UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);
    ~UniformBuffer() = default;

    UniformBuffer(UniformBuffer&&) noexcept = default;
    UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

    void* MapBuffer(GLenum access) const;
    void UnmapBuffer() const;

    [[nodiscard]] GLuint GetRendererID() const { return *m_RendererIDPtr; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }
    [[nodiscard]] GLsizeiptr GetSize() const { return m_Size; }

private:
    GLuint m_BindingPoint = 0;
    GLenum m_Usage = GL_DYNAMIC_DRAW;
    GLsizeiptr m_Size = 0;

    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr;
};