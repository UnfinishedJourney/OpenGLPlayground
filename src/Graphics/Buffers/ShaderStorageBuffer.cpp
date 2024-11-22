#include "ShaderStorageBuffer.h"
#include "Utilities/Logger.h"
#include <stdexcept>

ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage)
    : m_BindingPoint(bindingPoint), m_Size(size), m_Usage(usage), m_RendererIDPtr(new GLuint(0), BufferDeleter()) {
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        Logger::GetLogger()->error("Failed to create Shader Storage Buffer Object.");
        throw std::runtime_error("Failed to create Shader Storage Buffer Object.");
    }

    GLCall(glNamedBufferData(*m_RendererIDPtr, m_Size, nullptr, m_Usage));
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, *m_RendererIDPtr));

    Logger::GetLogger()->info("Created ShaderStorageBuffer with ID {} at binding point {}.", *m_RendererIDPtr, m_BindingPoint);
}

void ShaderStorageBuffer::Bind() const {
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_RendererIDPtr));
    Logger::GetLogger()->debug("Bound ShaderStorageBuffer ID: {} to binding point: {}", *m_RendererIDPtr, m_BindingPoint);
}

void ShaderStorageBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
    Logger::GetLogger()->debug("Unbound ShaderStorageBuffer from binding point: {}", m_BindingPoint);
}

void ShaderStorageBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) {
    if (size + offset > m_Size) {
        Logger::GetLogger()->error("Data size exceeds buffer capacity.");
        throw std::runtime_error("ShaderStorageBuffer::SetData size exceeds buffer capacity");
    }

    GLCall(glNamedBufferSubData(*m_RendererIDPtr, offset, size, data));
    Logger::GetLogger()->debug("Updated ShaderStorageBuffer ID: {} with new data.", *m_RendererIDPtr);
}

void ShaderStorageBuffer::BindBase() const {
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, *m_RendererIDPtr));
    Logger::GetLogger()->debug("Bound ShaderStorageBuffer ID: {} to binding point {}.", *m_RendererIDPtr, m_BindingPoint);
}