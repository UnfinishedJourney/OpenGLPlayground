#pragma once

#include <glad/glad.h>
#include <span>
#include <memory>
#include <cstddef>
#include "BufferDeleter.h"

class IndirectBuffer {
public:
    IndirectBuffer(std::span<const std::byte> data, GLenum usage = GL_DYNAMIC_DRAW);
    ~IndirectBuffer() = default;

    IndirectBuffer(const IndirectBuffer&) = delete;
    IndirectBuffer& operator=(const IndirectBuffer&) = delete;

    IndirectBuffer(IndirectBuffer&&) noexcept = default;
    IndirectBuffer& operator=(IndirectBuffer&&) noexcept = default;

    void Bind() const;
    void Unbind() const;

    void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

    GLuint GetRendererID() const { return *m_RendererIDPtr; }

private:
    std::unique_ptr<GLuint, BufferDeleter> m_RendererIDPtr = std::unique_ptr<GLuint, BufferDeleter>(new GLuint(0), BufferDeleter());
};