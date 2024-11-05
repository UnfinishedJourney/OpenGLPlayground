#include "VertexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
    : m_Usage(usage) {
    GLCall(glCreateBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        throw std::runtime_error("Failed to create Vertex Buffer Object.");
    }

    GLCall(glNamedBufferData(m_RendererID, data.size_bytes(), data.data(), m_Usage));
    Logger::GetLogger()->info("Created VertexBuffer with ID {}.", m_RendererID);
}

VertexBuffer::~VertexBuffer() {
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
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
            GLCall(glDeleteBuffers(1, &m_RendererID));
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
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) {
    GLCall(glNamedBufferSubData(m_RendererID, offset, data.size_bytes(), data.data()));
    Logger::GetLogger()->debug("Updated VertexBuffer ID: {} with new data.", m_RendererID);
}