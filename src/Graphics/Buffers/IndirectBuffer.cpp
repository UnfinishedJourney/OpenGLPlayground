#include "IndirectBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    IndirectBuffer::IndirectBuffer(std::span<const std::byte> data, GLenum usage)
        : m_BufferSize(data.size_bytes())
    {
        GLCall(glGenBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            throw std::runtime_error("Failed to create Indirect Buffer Object.");
        }
        Bind();
        GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, m_BufferSize, data.data(), usage));
        Unbind();
        Logger::GetLogger()->info("Created IndirectBuffer with ID={} Size={} bytes.", m_RendererID, m_BufferSize);
    }

    IndirectBuffer::IndirectBuffer() {
        GLCall(glGenBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            throw std::runtime_error("Failed to create Indirect Buffer Object.");
        }
        m_BufferSize = 0;
    }

    IndirectBuffer::~IndirectBuffer() {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted IndirectBuffer with ID={}.", m_RendererID);
        }
    }

    IndirectBuffer::IndirectBuffer(IndirectBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID), m_BufferSize(other.m_BufferSize)
    {
        other.m_RendererID = 0;
        other.m_BufferSize = 0;
    }

    IndirectBuffer& IndirectBuffer::operator=(IndirectBuffer&& other) noexcept {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_RendererID = other.m_RendererID;
            m_BufferSize = other.m_BufferSize;
            other.m_RendererID = 0;
            other.m_BufferSize = 0;
        }
        return *this;
    }

    void IndirectBuffer::Bind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
    }

    void IndirectBuffer::Unbind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
    }

    void IndirectBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) {
        Bind();
        GLCall(glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offset, data.size_bytes(), data.data()));
        Unbind();
        Logger::GetLogger()->debug("Updated IndirectBuffer ID={} Offset={} Size={}.", m_RendererID, offset, data.size_bytes());
    }

    void IndirectBuffer::SetData(std::span<const std::byte> data, GLenum usage) {
        Bind();
        GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, data.size_bytes(), data.data(), usage));
        Unbind();
        m_BufferSize = data.size_bytes();
        Logger::GetLogger()->info("Set IndirectBuffer ID={} new data. Size={}.", m_RendererID, m_BufferSize);
    }

} // namespace Graphics