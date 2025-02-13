#include "ShaderStorageBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace graphics {

    ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage)
        : bindingPoint_(bindingPoint), size_(size), usage_(usage)
    {
        GLCall(glCreateBuffers(1, &rendererID_));
        if (rendererID_ == 0) {
            Logger::GetLogger()->error("Failed to create Shader Storage Buffer Object.");
            throw std::runtime_error("Failed to create Shader Storage Buffer Object.");
        }
        GLCall(glNamedBufferData(rendererID_, size_, nullptr, usage_));
        GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint_, rendererID_));
        Logger::GetLogger()->info("Created ShaderStorageBuffer (ID={}) at binding point={} with size={} bytes.",
            rendererID_, bindingPoint_, size_);
    }

    ShaderStorageBuffer::~ShaderStorageBuffer()
    {
        if (rendererID_ != 0) {
            GLCall(glDeleteBuffers(1, &rendererID_));
            Logger::GetLogger()->info("Deleted ShaderStorageBuffer (ID={}).", rendererID_);
        }
    }

    ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept
        : bindingPoint_(other.bindingPoint_),
        size_(other.size_),
        usage_(other.usage_),
        rendererID_(other.rendererID_)
    {
        other.rendererID_ = 0;
    }

    ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept
    {
        if (this != &other) {
            if (rendererID_ != 0) {
                GLCall(glDeleteBuffers(1, &rendererID_));
            }
            bindingPoint_ = other.bindingPoint_;
            size_ = other.size_;
            usage_ = other.usage_;
            rendererID_ = other.rendererID_;
            other.rendererID_ = 0;
        }
        return *this;
    }

    void ShaderStorageBuffer::Bind() const
    {
        GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint_, rendererID_));
        Logger::GetLogger()->debug("Bound ShaderStorageBuffer (ID={}) to binding point {}.", rendererID_, bindingPoint_);
    }

    void ShaderStorageBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        Logger::GetLogger()->debug("Unbound ShaderStorageBuffer from GL_SHADER_STORAGE_BUFFER.");
    }

    void ShaderStorageBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset)
    {
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(size_)) {
            Logger::GetLogger()->error("ShaderStorageBuffer::UpdateData error: offset({}) + size({}) exceeds buffer capacity ({} bytes).",
                offset, data.size_bytes(), size_);
            throw std::runtime_error("ShaderStorageBuffer::UpdateData - data exceeds buffer size");
        }
        GLCall(glNamedBufferSubData(rendererID_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated ShaderStorageBuffer (ID={}) offset={} size={} bytes.",
            rendererID_, offset, data.size_bytes());
    }

} // namespace graphics