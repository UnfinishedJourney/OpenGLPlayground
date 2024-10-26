#include "Graphics/Buffers/VertexBuffer.h"
#include "Utilities/Logger.h" 
#include "Utilities/Utility.h" 
#include <stdexcept>

VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
    : m_Usage(usage) {
    glCreateBuffers(1, &m_RendererID);
    if (m_RendererID == 0) {
        throw std::runtime_error("Failed to create Vertex Buffer Object.");
    }

    glNamedBufferData(m_RendererID, data.size_bytes(), data.data(), m_Usage);
    Logger::GetLogger()->info("Created VertexBuffer with ID {}.", m_RendererID);
}

VertexBuffer::~VertexBuffer() {
    if (m_RendererID != 0) {
        glDeleteBuffers(1, &m_RendererID);
        Logger::GetLogger()->info("Deleted VertexBuffer with ID {}.", m_RendererID);
    }
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Usage(other.m_Usage) {
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved VertexBuffer with ID {}.", m_RendererID);
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            glDeleteBuffers(1, &m_RendererID);
            Logger::GetLogger()->info("Deleted VertexBuffer with ID {}.", m_RendererID);
        }
        m_RendererID = other.m_RendererID;
        m_Usage = other.m_Usage;
        other.m_RendererID = 0;
        Logger::GetLogger()->debug("Assigned VertexBuffer with ID {}.", m_RendererID);
    }
    return *this;
}

void VertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}