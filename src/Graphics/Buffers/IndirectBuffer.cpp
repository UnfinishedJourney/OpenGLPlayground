#include "IndirectBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndirectBuffer::IndirectBuffer(std::span<const std::byte> data, GLenum usage)
    : m_RendererIDPtr(std::make_unique<GLuint>(0).release(), BufferDeleter())
{
    // Note: Alternatively glCreateBuffers can be used if you prefer DSA style
    GLCall(glGenBuffers(1, m_RendererIDPtr.get()));
    if (*m_RendererIDPtr == 0) {
        throw std::runtime_error("Failed to create Indirect Buffer Object.");
    }

    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *m_RendererIDPtr));
    GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, data.size_bytes(), data.data(), usage));
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

    Logger::GetLogger()->info("Created IndirectBuffer with ID={}. Size={} bytes.",
        *m_RendererIDPtr, data.size_bytes());
}

void IndirectBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *m_RendererIDPtr));
}

void IndirectBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
}

void IndirectBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset)
{
    Bind();
    GLCall(glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offset, data.size_bytes(), data.data()));
    Unbind();
    Logger::GetLogger()->debug(
        "Updated IndirectBuffer ID={} with new data. Offset={}. Size={}",
        *m_RendererIDPtr, offset, data.size_bytes()
    );
}