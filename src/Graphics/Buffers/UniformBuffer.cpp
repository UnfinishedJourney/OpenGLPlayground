#include "UniformBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage)
        : m_BindingPoint(bindingPoint), m_Usage(usage), m_Size(size)
    {
        GLCall(glCreateBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
            throw std::runtime_error("Failed to create Uniform Buffer Object.");
        }
        GLCall(glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage));
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->info("Created UniformBuffer (ID={}) at binding point={} with size={}.",
            m_RendererID, m_BindingPoint, m_Size);
    }

    UniformBuffer::~UniformBuffer() {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted UniformBuffer (ID={}).", m_RendererID);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : m_BindingPoint(other.m_BindingPoint),
        m_Usage(other.m_Usage),
        m_Size(other.m_Size),
        m_RendererID(other.m_RendererID)
    {
        other.m_RendererID = 0;
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_BindingPoint = other.m_BindingPoint;
            m_Usage = other.m_Usage;
            m_Size = other.m_Size;
            m_RendererID = other.m_RendererID;
            other.m_RendererID = 0;
        }
        return *this;
    }

    void UniformBuffer::Bind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->debug("Bound UniformBuffer ID={} to binding point={}.", m_RendererID, m_BindingPoint);
    }

    void UniformBuffer::Unbind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0));
        Logger::GetLogger()->debug("Unbound UniformBuffer from binding point={}.", m_BindingPoint);
    }

    void UniformBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) const {
        // Check alignment for std140 (assumed 16-byte multiples).
        if ((offset % 16 != 0) || (size % 16 != 0)) {
            Logger::GetLogger()->error("UniformBuffer::SetData alignment error: offset={} and size={} must be multiples of 16.", offset, size);
            throw std::runtime_error("UniformBuffer::SetData alignment error");
        }
        if (offset + size > m_Size) {
            Logger::GetLogger()->error("UniformBuffer::SetData out of range: offset({}) + size({}) > buffer size({}).", offset, size, m_Size);
            throw std::out_of_range("UniformBuffer::SetData out of range");
        }
        GLCall(glNamedBufferSubData(m_RendererID, offset, size, data));
        Logger::GetLogger()->debug("Updated UniformBuffer ID={} offset={} size={}.", m_RendererID, offset, size);
    }

    void* UniformBuffer::MapBuffer(GLenum access) const {
        void* ptr = glMapNamedBuffer(m_RendererID, access);
        if (!ptr) {
            Logger::GetLogger()->error("Failed to map UniformBuffer ID={}.", m_RendererID);
        }
        return ptr;
    }

    void UniformBuffer::UnmapBuffer() const {
        GLCall(glUnmapNamedBuffer(m_RendererID));
        Logger::GetLogger()->debug("Unmapped UniformBuffer ID={}.", m_RendererID);
    }

} // namespace Graphics