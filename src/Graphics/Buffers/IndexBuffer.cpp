#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndexBuffer::IndexBuffer(std::span<const GLuint> data)
    : m_Count(static_cast<GLsizei>(data.size())) {
    Logger::GetLogger()->info("Creating IndexBuffer with {} indices.", m_Count);

    GLCall(glCreateBuffers(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Index Buffer Object.");
        throw std::runtime_error("Failed to create Index Buffer Object.");
    }

    GLCall(glNamedBufferData(m_RendererID, m_Count * sizeof(GLuint), data.data(), GL_STATIC_DRAW));
    Logger::GetLogger()->debug("Uploaded {} bytes of index data to Buffer ID {}.", m_Count * sizeof(GLuint), m_RendererID);
}

IndexBuffer::~IndexBuffer() {
    if (m_RendererID != 0) {
        GLCall(glDeleteBuffers(1, &m_RendererID));
        Logger::GetLogger()->info("Deleted IndexBuffer with Renderer ID {}.", m_RendererID);
    }
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_RendererID(other.m_RendererID), m_Count(other.m_Count) {
    other.m_RendererID = 0;
    other.m_Count = 0;
    Logger::GetLogger()->debug("Moved IndexBuffer with ID {}.", m_RendererID);
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted IndexBuffer with Renderer ID {}.", m_RendererID);
        }
        m_RendererID = other.m_RendererID;
        m_Count = other.m_Count;
        other.m_RendererID = 0;
        other.m_Count = 0;
        Logger::GetLogger()->debug("Assigned IndexBuffer with ID {}.", m_RendererID);
    }
    return *this;
}