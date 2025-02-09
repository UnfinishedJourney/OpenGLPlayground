#include "ShaderStorageBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage)
        : m_BindingPoint(bindingPoint), m_Size(size), m_Usage(usage)
    {
        GLCall(glCreateBuffers(1, &m_RendererID));
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to create Shader Storage Buffer Object.");
            throw std::runtime_error("Failed to create Shader Storage Buffer Object.");
        }
        GLCall(glNamedBufferData(m_RendererID, m_Size, nullptr, m_Usage));
        GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->info("Created ShaderStorageBuffer ID={} at binding point={} with size={} bytes.",
            m_RendererID, m_BindingPoint, m_Size);
    }

    ShaderStorageBuffer::~ShaderStorageBuffer() {
        if (m_RendererID != 0) {
            GLCall(glDeleteBuffers(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted ShaderStorageBuffer ID={}.", m_RendererID);
        }
    }

    ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept
        : m_BindingPoint(other.m_BindingPoint), m_Size(other.m_Size),
        m_Usage(other.m_Usage), m_RendererID(other.m_RendererID)
    {
        other.m_RendererID = 0;
    }

    ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteBuffers(1, &m_RendererID));
            }
            m_BindingPoint = other.m_BindingPoint;
            m_Size = other.m_Size;
            m_Usage = other.m_Usage;
            m_RendererID = other.m_RendererID;
            other.m_RendererID = 0;
        }
        return *this;
    }

    void ShaderStorageBuffer::Bind() const {
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID));
        Logger::GetLogger()->debug("Bound SSBO ID={} to GL_SHADER_STORAGE_BUFFER.", m_RendererID);
    }

    void ShaderStorageBuffer::Unbind() const {
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        Logger::GetLogger()->debug("Unbound SSBO from GL_SHADER_STORAGE_BUFFER.");
    }

    void ShaderStorageBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) {
        if (offset + size > m_Size) {
            Logger::GetLogger()->error("Data size exceeds SSBO capacity. Offset={} Size={} BufferSize={}",
                offset, size, m_Size);
            throw std::runtime_error("ShaderStorageBuffer::SetData - data exceeds buffer size");
        }
        GLCall(glNamedBufferSubData(m_RendererID, offset, size, data));
        Logger::GetLogger()->debug("Updated SSBO ID={} with new data. Offset={} Size={}.", m_RendererID, offset, size);
    }

    void ShaderStorageBuffer::BindBase() const {
        GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_RendererID));
        Logger::GetLogger()->debug("Bound SSBO ID={} to binding point={}.", m_RendererID, m_BindingPoint);
    }

} // namespace Graphics