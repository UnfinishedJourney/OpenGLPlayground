#include "VertexArray.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

VertexArray::VertexArray() {
    GLCall(glCreateVertexArrays(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to create Vertex Array Object.");
        throw std::runtime_error("Failed to create Vertex Array Object.");
    }
    Logger::GetLogger()->info("Created VertexArray with ID {}.", m_RendererID);
}

VertexArray::~VertexArray() {
    if (m_RendererID != 0) {
        GLCall(glDeleteVertexArrays(1, &m_RendererID));
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
            GLCall(glDeleteVertexArrays(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted VertexArray with ID {}.", m_RendererID);
        }
        m_RendererID = other.m_RendererID;
        other.m_RendererID = 0;
        Logger::GetLogger()->debug("Assigned VertexArray with ID {}.", m_RendererID);
    }
    return *this;
}

void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, GLuint bindingIndex) {
    GLCall(glVertexArrayVertexBuffer(m_RendererID, bindingIndex, vertexBuffer.GetRendererID(), 0, layout.GetStride()));

    const auto& elements = layout.GetElements();
    for (const auto& element : elements) {
        GLuint attribIndex = element.attributeIndex;
        GLCall(glEnableVertexArrayAttrib(m_RendererID, attribIndex));
        GLCall(glVertexArrayAttribFormat(m_RendererID, attribIndex, element.count, element.type, element.normalized, element.offset));
        GLCall(glVertexArrayAttribBinding(m_RendererID, attribIndex, bindingIndex));
    }
    Logger::GetLogger()->info("Added buffer to VertexArray ID {} with binding index {}.", m_RendererID, bindingIndex);
}

void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer) {
    GLCall(glVertexArrayElementBuffer(m_RendererID, indexBuffer.GetRendererID()));
    Logger::GetLogger()->info("Set IndexBuffer ID {} to VertexArray ID {}.", indexBuffer.GetRendererID(), m_RendererID);
}

void VertexArray::Bind() const {
    GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const {
    GLCall(glBindVertexArray(0));
}