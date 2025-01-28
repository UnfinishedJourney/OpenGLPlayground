#include "ShaderStorageBuffer.h"
#include <stdexcept>

ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage)
    : m_BindingPoint(bindingPoint)
    , m_Size(size)
    , m_Usage(usage)
    , m_RendererIDPtr(std::make_unique<GLuint>(0).release(), BufferDeleter())
{
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        Logger::GetLogger()->error("Failed to create Shader Storage Buffer Object.");
        throw std::runtime_error("Failed to create Shader Storage Buffer Object.");
    }

    // Allocate storage (nullptr data initially)
    GLCall(glNamedBufferData(*m_RendererIDPtr, m_Size, nullptr, m_Usage));
    // Automatically bind the buffer to the specified binding point
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, *m_RendererIDPtr));

    Logger::GetLogger()->info(
        "Created ShaderStorageBuffer ID={} at binding point={}. Size={} bytes.",
        *m_RendererIDPtr, m_BindingPoint, m_Size
    );
}

void ShaderStorageBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_RendererIDPtr));
    Logger::GetLogger()->debug("Bound SSBO ID={} to GL_SHADER_STORAGE_BUFFER.", *m_RendererIDPtr);
}

void ShaderStorageBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
    Logger::GetLogger()->debug("Unbound SSBO from GL_SHADER_STORAGE_BUFFER.");
}

void ShaderStorageBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset)
{
    if (offset + size > m_Size) {
        Logger::GetLogger()->error("Data size exceeds SSBO capacity. Offset={} Size={} BufferSize={}",
            offset, size, m_Size);
        throw std::runtime_error("ShaderStorageBuffer::SetData - data exceeds buffer size");
    }

    GLCall(glNamedBufferSubData(*m_RendererIDPtr, offset, size, data));
    Logger::GetLogger()->debug("Updated SSBO ID={} with new data. Offset={} Size={}",
        *m_RendererIDPtr, offset, size);
}

void ShaderStorageBuffer::BindBase() const
{
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, *m_RendererIDPtr));
    Logger::GetLogger()->debug("Bound SSBO ID={} to binding point={}.",
        *m_RendererIDPtr, m_BindingPoint);
}