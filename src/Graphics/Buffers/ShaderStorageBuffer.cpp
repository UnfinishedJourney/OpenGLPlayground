#include "ShaderStorageBuffer.h"
#include "Utilities/Utility.h" 
#include <stdexcept>

ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage)
    : m_BindingPoint(bindingPoint), m_Size(size), m_Usage(usage) {
    GLCall(glGenBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        throw std::runtime_error("Failed to create Shader Storage Buffer Object.");
    }

    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID));
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, m_Size, nullptr, m_Usage));
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_RendererID));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    Logger::GetLogger()->info("Created ShaderStorageBuffer with ID {} at binding point {}.", m_RendererID, m_BindingPoint);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted ShaderStorageBuffer with ID {}.", m_RendererID);
    }
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID),
    m_BindingPoint(other.m_BindingPoint),
    m_Size(other.m_Size),
    m_Usage(other.m_Usage) {
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved ShaderStorageBuffer with ID {}.", m_RendererID);
}

ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted ShaderStorageBuffer with ID {}.", m_RendererID);
        }

        m_RendererID = other.m_RendererID;
        m_BindingPoint = other.m_BindingPoint;
        m_Size = other.m_Size;
        m_Usage = other.m_Usage;

        other.m_RendererID = 0;
        Logger::GetLogger()->debug("Assigned ShaderStorageBuffer with ID {}.", m_RendererID);
    }
    return *this;
}

void ShaderStorageBuffer::Bind() const {
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID));
}

void ShaderStorageBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void ShaderStorageBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) {
    if (size + offset > m_Size) {
        throw std::runtime_error("Data size exceeds buffer capacity.");
    }

    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID));
    GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    Logger::GetLogger()->debug("Updated ShaderStorageBuffer ID {} with new data.", m_RendererID);
}