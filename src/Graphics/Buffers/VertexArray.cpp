#include "Graphics/Buffers/VertexArray.h"
#include "Utilities/Logger.h" 

VertexArray::VertexArray()
    : m_RendererID(0)
{
    Logger::GetLogger()->info("Creating new VertexArray.");

    GLCall(glGenVertexArrays(1, &m_RendererID));
    if (m_RendererID == 0) {
        Logger::GetLogger()->error("Failed to generate Vertex Array Object (VAO).");
        throw std::runtime_error("Failed to generate Vertex Array Object (VAO).");
    }
    Logger::GetLogger()->info("Generated VertexArray with ID {}.", m_RendererID);
}

VertexArray::VertexArray(unsigned int rendererID)
    : m_RendererID(rendererID)
{
    Logger::GetLogger()->info("Creating VertexArray with existing Renderer ID {}.", rendererID);
}

VertexArray::~VertexArray()
{
    Logger::GetLogger()->info("Deleting VertexArray with ID {}.", m_RendererID);
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_RendererID(other.m_RendererID)
{
    other.m_RendererID = 0;
    Logger::GetLogger()->info("Moved VertexArray with ID {}.", m_RendererID);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other) {
        // Release current resource
        if (m_RendererID != 0) {
            GLCall(glDeleteVertexArrays(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted VertexArray with ID {}.", m_RendererID);
        }

        // Move resources
        m_RendererID = other.m_RendererID;
        other.m_RendererID = 0;
        Logger::GetLogger()->info("Assigned VertexArray with ID {}.", m_RendererID);
    }
    return *this;
}

void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout) const
{
    Logger::GetLogger()->info("Adding VertexBuffer ID {} to VertexArray ID {}.", vertexBuffer.GetRendererID(), m_RendererID);

    Bind();
    vertexBuffer.Bind();
    const auto& elements = vbLayout.GetElements();
    uintptr_t offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type,
            element.normalized, vbLayout.GetStride(), reinterpret_cast<const void*>(offset)));
        Logger::GetLogger()->debug("Configured Vertex Attribute {}: count={}, type={}, normalized={}.",
            i, element.count, element.type, element.normalized);
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }

    Unbind();
    Logger::GetLogger()->info("Successfully added VertexBuffer ID {} to VertexArray ID {}.", vertexBuffer.GetRendererID(), m_RendererID);
}

void VertexArray::AddInstancedBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& vbLayout, unsigned int divisor) const
{
    Logger::GetLogger()->info("Adding Instanced VertexBuffer ID {} to VertexArray ID {} with divisor {}.",
        vertexBuffer.GetRendererID(), m_RendererID, divisor);

    Bind();
    vertexBuffer.Bind();
    const auto& elements = vbLayout.GetElements();
    uintptr_t offset = 0;

    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type,
            element.normalized, vbLayout.GetStride(), reinterpret_cast<const void*>(offset)));
        GLCall(glVertexAttribDivisor(i, divisor));
        Logger::GetLogger()->debug("Configured Instanced Vertex Attribute {}: divisor={}.", i, divisor);
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }

    Unbind();
    Logger::GetLogger()->info("Successfully added Instanced VertexBuffer ID {} to VertexArray ID {}.", vertexBuffer.GetRendererID(), m_RendererID);
}

void VertexArray::Bind() const
{
    GLCall(glBindVertexArray(m_RendererID));
    Logger::GetLogger()->debug("Bound VertexArray ID {}.", m_RendererID);
}

void VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
    Logger::GetLogger()->debug("Unbound VertexArray.");
}