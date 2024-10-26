#include "Graphics/Buffers/VertexArray.h"
#include "Utilities/Logger.h"
#include <stdexcept>

VertexArray::VertexArray() {
    glCreateVertexArrays(1, &m_RendererID);
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Vertex Array Object.");
        throw std::runtime_error("Failed to create Vertex Array Object.");
    }
    Logger::GetLogger()->info("Created VertexArray with ID {}.", m_RendererID);
}

VertexArray::~VertexArray() {
    if (m_RendererID != 0) {
        glDeleteVertexArrays(1, &m_RendererID);
        Logger::GetLogger()->info("Deleted VertexArray with ID {}.", m_RendererID);
    }
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_RendererID(other.m_RendererID) {
    other.m_RendererID = 0;
    Logger::GetLogger()->debug("Moved VertexArray with ID {}.", m_RendererID);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        if (m_RendererID != 0) {
            glDeleteVertexArrays(1, &m_RendererID);
            Logger::GetLogger()->info("Deleted VertexArray with ID {}.", m_RendererID);
        }
        m_RendererID = other.m_RendererID;
        other.m_RendererID = 0;
        Logger::GetLogger()->debug("Assigned VertexArray with ID {}.", m_RendererID);
    }
    return *this;
}

void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout) {
    glVertexArrayVertexBuffer(m_RendererID, 0, vertexBuffer.GetRendererID(), 0, layout.GetStride());

    const auto& elements = layout.GetElements();
    GLuint index = 0;
    for (const auto& element : elements) {
        glEnableVertexArrayAttrib(m_RendererID, index);
        glVertexArrayAttribFormat(m_RendererID, index, element.count, element.type, element.normalized, element.offset);
        glVertexArrayAttribBinding(m_RendererID, index, 0);
        ++index;
    }
    Logger::GetLogger()->info("Added buffer to VertexArray ID {}.", m_RendererID);
}

void VertexArray::Bind() const {
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}