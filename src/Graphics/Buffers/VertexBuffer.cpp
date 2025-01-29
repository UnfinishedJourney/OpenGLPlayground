#include "VertexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // GLCall macros
#include <stdexcept>

VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
    : m_Size(data.size_bytes())
    , m_Usage(usage)
    , m_RendererIDPtr(std::make_unique<GLuint>(0).release(), BufferDeleter())
{
    // Create the buffer object
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        throw std::runtime_error("Failed to create Vertex Buffer Object.");
    }

    // Allocate and upload initial data
    GLCall(glNamedBufferData(*m_RendererIDPtr, m_Size, data.data(), m_Usage));

    Logger::GetLogger()->info(
        "Created VertexBuffer (ID={}) with size={} bytes.",
        *m_RendererIDPtr, m_Size
    );
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, *m_RendererIDPtr));
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset)
{
    if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(m_Size)) {
        throw std::runtime_error("VertexBuffer::UpdateData: Data exceeds buffer size.");
    }

    GLCall(glNamedBufferSubData(*m_RendererIDPtr, offset, data.size_bytes(), data.data()));
    Logger::GetLogger()->debug(
        "Updated VertexBuffer (ID={}) offset={} size={}.",
        *m_RendererIDPtr, offset, data.size_bytes()
    );
}