#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
    : m_Count(static_cast<GLsizei>(data.size())), m_Usage(usage), m_RendererIDPtr(new GLuint(0), BufferDeleter()) {
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        throw std::runtime_error("Failed to create Index Buffer Object.");
    }
    m_RendererID = *m_RendererIDPtr;

    GLCall(glNamedBufferData(m_RendererID, data.size_bytes(), data.data(), m_Usage));
    Logger::GetLogger()->info("Created IndexBuffer with ID {}.", m_RendererID);
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