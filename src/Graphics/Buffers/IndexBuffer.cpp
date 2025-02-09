#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
        : m_Count(static_cast<GLsizei>(data.size())),
        m_Size(data.size_bytes()),
        m_Usage(usage)
    {
        GLCall(glCreateBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            throw std::runtime_error("Failed to create Index Buffer Object.");
        }
        GLCall(glNamedBufferData(m_RendererID, m_Size, data.data(), m_Usage));
        Logger::GetLogger()->info("Created IndexBuffer (ID={}) with Count={}.", m_RendererID, m_Count);
    }

    IndexBuffer::~IndexBuffer()
    {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted IndexBuffer (ID={}).", m_RendererID);
        }
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID),
        m_Count(other.m_Count),
        m_Size(other.m_Size),
        m_Usage(other.m_Usage)
    {
        other.m_RendererID = 0;
        other.m_Count = 0;
        other.m_Size = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
    {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_RendererID = other.m_RendererID;
            m_Count = other.m_Count;
            m_Size = other.m_Size;
            m_Usage = other.m_Usage;
            other.m_RendererID = 0;
            other.m_Count = 0;
            other.m_Size = 0;
        }
        return *this;
    }

    void IndexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    }

    void IndexBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void IndexBuffer::UpdateData(std::span<const GLuint> data, GLintptr offset)
    {
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(m_Size)) {
            throw std::runtime_error("IndexBuffer::UpdateData: Data exceeds buffer size.");
        }
        // Optionally update m_Count if the entire buffer is replaced.
        m_Count = static_cast<GLsizei>(data.size());
        GLCall(glNamedBufferSubData(m_RendererID, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated IndexBuffer (ID={}) at offset={} with new data.", m_RendererID, offset);
    }

} // namespace Graphics