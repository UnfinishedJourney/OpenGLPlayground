#include "Graphics/Buffers/UniformBuffer.h"
#include "Utilities/Utility.h"

#include <stdexcept>

UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage)
    : m_BindingPoint(bindingPoint), m_Usage(usage), m_Size(size) {
    glCreateBuffers(1, &m_RendererID);
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
        throw std::runtime_error("Failed to create Uniform Buffer Object.");
    }

    glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);

    Logger::GetLogger()->info("Created UniformBuffer ID: {} with binding point: {} and size: {}", m_RendererID, m_BindingPoint, m_Size);
}

UniformBuffer::~UniformBuffer() {
    if (m_RendererID != 0) {
        glDeleteBuffers(1, &m_RendererID);
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
            glDeleteBuffers(1, &m_RendererID);
            Logger::GetLogger()->info("Deleted UniformBuffer ID: {}", m_RendererID);
        }

        m_RendererID = other.m_RendererID;
        m_BindingPoint = other.m_BindingPoint;
        m_Usage = other.m_Usage;
        m_Size = other.m_Size;

        other.m_RendererID = 0;

        Logger::GetLogger()->debug("Move-assigned UniformBuffer. New ID: {}", m_RendererID);
    }
    return *this;
}

void UniformBuffer::Bind() const {
    glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);
    Logger::GetLogger()->debug("Bound UniformBuffer ID: {} to binding point: {}", m_RendererID, m_BindingPoint);
}

void UniformBuffer::Unbind() const {
    glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0);
    Logger::GetLogger()->debug("Unbound UniformBuffer from binding point: {}", m_BindingPoint);
}