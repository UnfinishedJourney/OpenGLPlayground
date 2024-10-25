#include "Graphics/Buffers/VertexBuffer.h"
#include "Utilities/Logger.h" 
#include "Utilities/Utility.h" 
#include <stdexcept>

VertexBuffer::VertexBuffer(const void* data, unsigned int size, GLenum target, GLenum usage)
    : m_RendererID(0), m_Target(target), m_Usage(usage)
{
    GLCall(glGenBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        throw std::runtime_error("Failed to generate Vertex Buffer Object (VBO).");
    }

    GLCall(glBindBuffer(m_Target, m_RendererID));
    GLCall(glBufferData(m_Target, size, data, m_Usage));

    Unbind(); 
}

VertexBuffer::~VertexBuffer()
{
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
    }
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Target(other.m_Target), m_Usage(other.m_Usage)
{
    other.m_RendererID = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other) {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
        }

        m_RendererID = other.m_RendererID;
        m_Target = other.m_Target;
        m_Usage = other.m_Usage;

        other.m_RendererID = 0;
    }
    return *this;
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(m_Target, m_RendererID));
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(m_Target, 0));
}