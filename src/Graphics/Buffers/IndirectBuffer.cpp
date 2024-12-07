#include "IndirectBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

IndirectBuffer::IndirectBuffer(std::span<const std::byte> data, GLenum usage) {
    GLCall(glGenBuffers(1, m_RendererIDPtr.get()));
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *m_RendererIDPtr));
    GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, data.size_bytes(), data.data(), usage));
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
}

void IndirectBuffer::Bind() const {
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *m_RendererIDPtr));
}

void IndirectBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
}

void IndirectBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) {
    Bind();
    GLCall(glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offset, data.size_bytes(), data.data()));
    Unbind();
}