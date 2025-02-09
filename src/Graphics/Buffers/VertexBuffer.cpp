#include "VertexBuffer.h"
#include "Utilities/Logger.h"   // Your logging utility
#include "Utilities/Utility.h"  // Contains GLCall macro for error checking
#include <stdexcept>

namespace Graphics {

    VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
        : m_Size(data.size_bytes()), m_Usage(usage)
    {
        // Create the buffer
        GLCall(glCreateBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            throw std::runtime_error("Failed to create Vertex Buffer Object.");
        }
        // Allocate and upload data using Direct State Access (DSA)
        GLCall(glNamedBufferData(m_RendererID, m_Size, data.data(), m_Usage));

        Logger::GetLogger()->info("Created VertexBuffer (ID={}) with size={} bytes.", m_RendererID, m_Size);
    }

    VertexBuffer::~VertexBuffer()
    {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted VertexBuffer (ID={}).", m_RendererID);
        }
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID),
        m_Size(other.m_Size),
        m_Usage(other.m_Usage)
    {
        other.m_RendererID = 0;
        other.m_Size = 0;
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
    {
        if (this != &other) {
            // Release any current resource.
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_RendererID = other.m_RendererID;
            m_Size = other.m_Size;
            m_Usage = other.m_Usage;
            other.m_RendererID = 0;
            other.m_Size = 0;
        }
        return *this;
    }

    void VertexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
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
        GLCall(glNamedBufferSubData(m_RendererID, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated VertexBuffer (ID={}) offset={} size={}.", m_RendererID, offset, data.size_bytes());
    }

} // namespace Graphics