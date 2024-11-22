#include "VertexArray.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <algorithm>

VertexArray::VertexArray()
    : m_RendererIDPtr(new GLuint(0), VertexArrayDeleter()) {
    GLCall(glCreateVertexArrays(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        Logger::GetLogger()->error("Failed to create Vertex Array Object.");
        throw std::runtime_error("Failed to create Vertex Array Object.");
    }
    Logger::GetLogger()->info("Created VertexArray with ID {}.", *m_RendererIDPtr);
}

void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, GLuint bindingIndex) {
    if (std::find(m_BindingIndices.begin(), m_BindingIndices.end(), bindingIndex) != m_BindingIndices.end()) {
        Logger::GetLogger()->error("Binding index {} is already used in VertexArray ID {}.", bindingIndex, *m_RendererIDPtr);
        throw std::invalid_argument("Duplicate binding index in VertexArray::AddBuffer");
    }

    GLCall(glVertexArrayVertexBuffer(*m_RendererIDPtr, bindingIndex, vertexBuffer.GetRendererID(), 0, layout.GetStride()));

    const auto& elements = layout.GetElements();
    for (const auto& element : elements) {
        GLuint attribIndex = element.attributeIndex;
        GLCall(glEnableVertexArrayAttrib(*m_RendererIDPtr, attribIndex));
        GLCall(glVertexArrayAttribFormat(*m_RendererIDPtr, attribIndex, element.count, element.type, element.normalized, element.offset));
        GLCall(glVertexArrayAttribBinding(*m_RendererIDPtr, attribIndex, bindingIndex));
    }

    m_BindingIndices.push_back(bindingIndex);
    Logger::GetLogger()->info("Added buffer to VertexArray ID {} with binding index {}.", *m_RendererIDPtr, bindingIndex);
}

void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer) {
    GLCall(glVertexArrayElementBuffer(*m_RendererIDPtr, indexBuffer.GetRendererID()));
    Logger::GetLogger()->info("Set IndexBuffer ID {} to VertexArray ID {}.", indexBuffer.GetRendererID(), *m_RendererIDPtr);
}

void VertexArray::Bind() const {
    GLCall(glBindVertexArray(*m_RendererIDPtr));
}

void VertexArray::Unbind() const {
    GLCall(glBindVertexArray(0));
}