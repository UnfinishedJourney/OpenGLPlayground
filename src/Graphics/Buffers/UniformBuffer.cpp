#include "UniformBuffer.h"
#include "Utilities/Utility.h"
#include <stdexcept>

UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage)
    : m_BindingPoint(bindingPoint), m_Usage(usage), m_Size(size) {
    GLCall(glCreateBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
        throw std::runtime_error("Failed to create Uniform Buffer Object.");
    }

    GLCall(glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage));
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));

    Logger::GetLogger()->info("Created UniformBuffer ID: {} with binding point: {} and size: {}", m_RendererID, m_BindingPoint, m_Size);
}

UniformBuffer::~UniformBuffer() {
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted UniformBuffer ID: {}", m_RendererID);
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_BindingPoint(other.m_BindingPoint), m_Usage(other.m_Usage), m_Size(other.m_Size) {
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved UniformBuffer. New ID: {}", m_RendererID);
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted UniformBuffer ID: {}", m_RendererID);
        }

        m_RendererID = other.m_RendererID;
        m_BindingPoint = other.m_BindingPoint;
        m_Usage = other.m_Usage;
        m_Size = other.m_Size;

        other.m_RendererID = 0;

        Logger::GetLogger()->debug("Assigned UniformBuffer. New ID: {}", m_RendererID);
    }
    return *this;
}

void UniformBuffer::Bind() const {
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
    Logger::GetLogger()->debug("Bound UniformBuffer ID: {} to binding point: {}", m_RendererID, m_BindingPoint);
}

void UniformBuffer::Unbind() const {
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0));
    Logger::GetLogger()->debug("Unbound UniformBuffer from binding point: {}", m_BindingPoint);
}

void UniformBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) const {
    // Ensure offset and size align with std140 requirements
    if ((offset % 16 != 0) || (size % 16 != 0)) {
        Logger::GetLogger()->error("Data alignment error: offset and size must be multiples of 16 bytes.");
        throw std::runtime_error("UniformBuffer::SetData alignment error");
    }
    if (offset + size > m_Size) {
        Logger::GetLogger()->error("SetData out of range: offset ({}) + size ({}) > buffer size ({})", offset, size, m_Size);
        throw std::out_of_range("UniformBuffer::SetData out of range");
    }
    GLCall(glNamedBufferSubData(m_RendererID, offset, size, data));
    Logger::GetLogger()->debug("Updated UniformBuffer ID: {} at offset: {} with size: {}", m_RendererID, offset, size);
}

void* UniformBuffer::MapBuffer(GLenum access) const {
    void* ptr = glMapNamedBuffer(m_RendererID, access);
    if (!ptr) {
        Logger::GetLogger()->error("Failed to map UniformBuffer ID: {}", m_RendererID);
    }
    return ptr;
}

void UniformBuffer::UnmapBuffer() const {
    GLCall(glUnmapNamedBuffer(m_RendererID));
}