#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage)
    : m_RendererID(0), m_BindingPoint(bindingPoint), m_Usage(usage), m_Size(size)
{
    GLCall(glCreateBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to generate Uniform Buffer Object (UBO).");
        throw std::runtime_error("Failed to generate Uniform Buffer Object (UBO).");
    }

    // Allocate buffer with specified size
    GLCall(glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage));
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));

    Logger::GetLogger()->info("Created UniformBuffer ID: {} with binding point: {} and size: {}", m_RendererID, m_BindingPoint, m_Size);
}

UniformBuffer::~UniformBuffer()
{
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted UniformBuffer ID: {}", m_RendererID);
    }
}

// Move Constructor
UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_BindingPoint(other.m_BindingPoint), m_Usage(other.m_Usage), m_Size(other.m_Size)
{
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved UniformBuffer. New ID: {}", m_RendererID);
}

// Move Assignment Operator
UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
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

        Logger::GetLogger()->debug("Move-assigned UniformBuffer. New ID: {}", m_RendererID);
    }
    return *this;
}

void UniformBuffer::Bind() const
{
    if (m_RendererID != 0) {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->debug("Bound UniformBuffer ID: {} to binding point: {}", m_RendererID, m_BindingPoint);
    }
    else {
        Logger::GetLogger()->error("Failed to bind UniformBuffer. RendererID is 0.");
    }
}

void UniformBuffer::Unbind() const
{
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0));
    Logger::GetLogger()->debug("Unbound UniformBuffer from binding point: {}", m_BindingPoint);
}
