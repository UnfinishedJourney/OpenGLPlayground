#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
    : m_Count(static_cast<GLsizei>(data.size()))
    , m_Usage(usage)
    , m_Size(data.size_bytes())
    , m_RendererIDPtr(std::make_unique<GLuint>(0).release(), BufferDeleter())
{
    GLCall(glCreateBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        throw std::runtime_error("Failed to create Index Buffer Object.");
    }


    GLCall(glNamedBufferData(*m_RendererIDPtr, m_Size, data.data(), m_Usage));
    Logger::GetLogger()->info("Created IndexBuffer with ID={}. Count={}",
        *m_RendererIDPtr, m_Count);
}

void IndexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *m_RendererIDPtr));
}

void IndexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::UpdateData(std::span<const GLuint> data, GLintptr offset)
{
    if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(m_Size)) {
        throw std::runtime_error(
            "IndexBuffer::UpdateData: Data exceeds buffer size."
        );
    }
    m_Count = static_cast<GLsizei>(data.size());
    GLCall(glNamedBufferSubData(*m_RendererIDPtr, offset, data.size_bytes(), data.data()));
    Logger::GetLogger()->debug(
        "Updated IndexBuffer ID={} with new data at offset={}.",
        *m_RendererIDPtr, offset
    );
}