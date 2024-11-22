#include "VertexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
    : m_Usage(usage), m_RendererIDPtr(new GLuint(0), BufferDeleter()) {
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        throw std::runtime_error("Failed to create Vertex Buffer Object.");
    }
    m_RendererID = *m_RendererIDPtr;

    GLCall(glNamedBufferData(m_RendererID, data.size_bytes(), data.data(), m_Usage));
    Logger::GetLogger()->info("Created VertexBuffer with ID {}.", m_RendererID);
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