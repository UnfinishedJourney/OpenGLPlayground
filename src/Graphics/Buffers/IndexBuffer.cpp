#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
    : m_Count(static_cast<GLsizei>(data.size())), m_Usage(usage) {
    GLCall(glCreateBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        throw std::runtime_error("Failed to create Index Buffer Object.");
    }

    GLCall(glNamedBufferData(m_RendererID, data.size_bytes(), data.data(), m_Usage));
    Logger::GetLogger()->info("Created IndexBuffer with ID {}.", m_RendererID);
}

IndexBuffer::~IndexBuffer() {
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted IndexBuffer with ID {}.", m_RendererID);
    }
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Count(other.m_Count), m_Usage(other.m_Usage) {
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved IndexBuffer with ID {}.", m_RendererID);
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted IndexBuffer with ID {}.", m_RendererID);
        }
        m_RendererID = other.m_RendererID;
        m_Count = other.m_Count;
        m_Usage = other.m_Usage;
        other.m_RendererID = 0;
        Logger::GetLogger()->debug("Assigned IndexBuffer with ID {}.", m_RendererID);
    }
    return *this;
}

void IndexBuffer::Bind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::UpdateData(std::span<const GLuint> data, GLintptr offset) {
    m_Count = static_cast<GLsizei>(data.size());
    GLCall(glNamedBufferSubData(m_RendererID, offset, data.size_bytes(), data.data()));
    Logger::GetLogger()->debug("Updated IndexBuffer ID: {} with new data.", m_RendererID);
}