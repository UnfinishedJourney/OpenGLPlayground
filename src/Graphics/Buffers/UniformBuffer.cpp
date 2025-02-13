#include "UniformBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace graphics {

    UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint,
        UniformBufferLayout layout, GLenum usage)
        : m_BindingPoint(bindingPoint), m_Usage(usage), m_Size(size), m_Layout(layout)
    {
        GLCall(glCreateBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
            throw std::runtime_error("Failed to create Uniform Buffer Object.");
        }
        // Allocate the buffer with no initial data.
        GLCall(glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage));
        // Bind the buffer to the specified uniform buffer binding point.
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->info("Created UniformBuffer (ID={}) at binding point={} with size={} bytes.",
            m_RendererID, m_BindingPoint, m_Size);
    }

    UniformBuffer::~UniformBuffer()
    {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted UniformBuffer (ID={}).", m_RendererID);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID),
        m_BindingPoint(other.m_BindingPoint),
        m_Usage(other.m_Usage),
        m_Size(other.m_Size),
        m_Layout(other.m_Layout)
    {
        other.m_RendererID = 0;
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
    {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_RendererID = other.m_RendererID;
            m_BindingPoint = other.m_BindingPoint;
            m_Usage = other.m_Usage;
            m_Size = other.m_Size;
            m_Layout = other.m_Layout;
            other.m_RendererID = 0;
        }
        return *this;
    }

    void UniformBuffer::Bind() const
    {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->debug("Bound UniformBuffer (ID={}) to binding point={}.", m_RendererID, m_BindingPoint);
    }

    void UniformBuffer::Unbind() const
    {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0));
        Logger::GetLogger()->debug("Unbound UniformBuffer from binding point={}.", m_BindingPoint);
    }

    void UniformBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) const
    {
        // Enforce alignment for Std140 (typically multiples of 16 bytes)
        if (m_Layout == UniformBufferLayout::Std140) {
            if ((offset % 16 != 0) || (data.size_bytes() % 16 != 0)) {
                Logger::GetLogger()->error("UniformBuffer::UpdateData alignment error: offset={} and size={} must be multiples of 16 (Std140).", offset, data.size_bytes());
                throw std::runtime_error("UniformBuffer::UpdateData alignment error");
            }
        }
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(m_Size)) {
            Logger::GetLogger()->error("UniformBuffer::UpdateData out of range: offset({}) + size({}) > buffer size({}).", offset, data.size_bytes(), m_Size);
            throw std::out_of_range("UniformBuffer::UpdateData out of range");
        }
        GLCall(glNamedBufferSubData(m_RendererID, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated UniformBuffer (ID={}) offset={} size={} bytes.", m_RendererID, offset, data.size_bytes());
    }

    void* UniformBuffer::MapBuffer(GLenum access) const
    {
        void* ptr = glMapNamedBuffer(m_RendererID, access);
        if (!ptr) {
            Logger::GetLogger()->error("Failed to map UniformBuffer (ID={}).", m_RendererID);
        }
        return ptr;
    }

    void UniformBuffer::UnmapBuffer() const
    {
        GLCall(glUnmapNamedBuffer(m_RendererID));
        Logger::GetLogger()->debug("Unmapped UniformBuffer (ID={}).", m_RendererID);
    }

} // namespace graphics