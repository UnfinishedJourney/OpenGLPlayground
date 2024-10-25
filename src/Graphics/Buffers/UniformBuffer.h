#pragma once

#include <glad/glad.h>
#include <glm.hpp>
#include <variant>
#include <string>
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"

class UniformBuffer {
public:
    // Define UniformValue using std::variant
    using UniformValue = std::variant<int, float, glm::vec3, glm::mat4>;

    // Constructor with explicit buffer size
    UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);

    ~UniformBuffer();

    // Delete copy constructor and assignment operator
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    // Move constructor and assignment operator
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    // Templated SetData method
    template <typename T>
    void SetData(const T& data, GLuint offset = 0) const;

    [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }
    [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }
    [[nodiscard]] GLsizeiptr GetSize() const { return m_Size; }

private:
    GLuint m_RendererID;
    GLuint m_BindingPoint;
    GLenum m_Usage;
    GLsizeiptr m_Size;
};

// Template method definitions must be in the header
template <typename T>
void UniformBuffer::SetData(const T& data, GLuint offset) const
{
    if (offset + sizeof(T) > static_cast<GLuint>(m_Size)) {
        Logger::GetLogger()->error(
            "SetData out of range: offset ({}) + size ({}) > buffer size ({})",
            offset, sizeof(T), m_Size
        );
        throw std::out_of_range("UniformBuffer::SetData out of range");
    }

    GLCall(glNamedBufferSubData(m_RendererID, offset, sizeof(T), &data));
    Logger::GetLogger()->debug(
        "Updated UniformBuffer ID: {} at offset: {} with size: {}",
        m_RendererID, offset, sizeof(T)
    );
}