#include "Graphics/Buffers/VertexBuffer.h"
#include "Utilities/Logger.h" 
#include "Utilities/Utility.h" 
#include <stdexcept>

VertexBuffer::VertexBuffer(const void* data, unsigned int size, GLenum target, GLenum usage)
    : m_RendererID(0), m_Target(target), m_Usage(usage)
{
    Logger::GetLogger()->info("Creating VertexBuffer. Target: {}, Usage: {}.", target, usage);

    GLCall(glGenBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Vertex Buffer Object (VBO).");
        throw std::runtime_error("Failed to generate Vertex Buffer Object (VBO).");
    }
    Logger::GetLogger()->info("Generated VertexBuffer with ID {}.", m_RendererID);

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    GLCall(glBufferData(m_Target, size, data, m_Usage));
    Logger::GetLogger()->info("Uploaded data to VertexBuffer ID {}. Size: {} bytes.", m_RendererID, size);
    Unbind();
}

VertexBuffer::~VertexBuffer()
{
    if (m_RendererID != 0) {
        Logger::GetLogger()->info("Deleting VertexBuffer with ID {}.", m_RendererID);
        GLCall(glDeleteBuffers(1, &m_RendererID));
    }
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Target(other.m_Target), m_Usage(other.m_Usage)
{
    other.m_RendererID = 0;
    Logger::GetLogger()->info("Moved VertexBuffer with ID {}.", m_RendererID);
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other) {
        // Release current resource
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted VertexBuffer with ID {}.", m_RendererID);
        }

        // Move resources
        m_RendererID = other.m_RendererID;
        m_Target = other.m_Target;
        m_Usage = other.m_Usage;

        other.m_RendererID = 0;
        Logger::GetLogger()->info("Assigned VertexBuffer with ID {}.", m_RendererID);
    }
    return *this;
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(m_Target, m_RendererID));
    Logger::GetLogger()->debug("Bound VertexBuffer ID {} to target {}.", m_RendererID, m_Target);
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(m_Target, 0));
    Logger::GetLogger()->debug("Unbound VertexBuffer from target {}.", m_Target);
}