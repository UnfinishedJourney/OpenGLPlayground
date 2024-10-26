#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <variant>
#include <span>
#include "Utilities/Logger.h"

class UniformBuffer {
public:
    using UniformValue = std::variant<int, float, glm::vec3, glm::mat4>;

    UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);
    ~UniformBuffer();

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    template <typename T>
    void SetData(const T& data, GLintptr offset = 0) const;

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }
    [[nodiscard]] GLsizeiptr GetSize() const { return m_Size; }

private:
    GLuint m_RendererID = 0;
    GLuint m_BindingPoint = 0;
    GLenum m_Usage = GL_DYNAMIC_DRAW;
    GLsizeiptr m_Size = 0;
};

template <typename T>
void UniformBuffer::SetData(const T& data, GLintptr offset) const {
    if (offset + sizeof(T) > m_Size) {
        Logger::GetLogger()->error("SetData out of range: offset ({}) + size ({}) > buffer size ({})", offset, sizeof(T), m_Size);
        throw std::out_of_range("UniformBuffer::SetData out of range");
    }
    glNamedBufferSubData(m_RendererID, offset, sizeof(T), &data);
    Logger::GetLogger()->debug("Updated UniformBuffer ID: {} at offset: {} with size: {}", m_RendererID, offset, sizeof(T));
}
