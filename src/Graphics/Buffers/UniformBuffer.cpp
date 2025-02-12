#include "UniformBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint binding_point, GLenum usage)
        : binding_point_{ binding_point }, usage_{ usage }, size_{ size }
    {
        GLCall(glCreateBuffers(1, &renderer_id_));
        if (renderer_id_ == 0) {
            Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
            throw std::runtime_error("Failed to create Uniform Buffer Object.");
        }
        // Allocate the buffer with no initial data.
        GLCall(glNamedBufferData(renderer_id_, size_, nullptr, usage_));
        // Bind the buffer to the specified uniform buffer binding point.
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, renderer_id_));
        Logger::GetLogger()->info("Created UniformBuffer (ID={}) at binding point={} with size={}.",
            renderer_id_, binding_point_, size_);
    }

    UniformBuffer::~UniformBuffer() {
        if (renderer_id_ != 0) {
            GLCall(glDeleteBuffers(1, &renderer_id_));
            Logger::GetLogger()->info("Deleted UniformBuffer (ID={}).", renderer_id_);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : binding_point_{ other.binding_point_ },
        usage_{ other.usage_ },
        size_{ other.size_ },
        renderer_id_{ other.renderer_id_ }
    {
        other.renderer_id_ = 0;
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
        if (this != &other) {
            if (renderer_id_ != 0) {
                GLCall(glDeleteBuffers(1, &renderer_id_));
            }
            binding_point_ = other.binding_point_;
            usage_ = other.usage_;
            size_ = other.size_;
            renderer_id_ = other.renderer_id_;
            other.renderer_id_ = 0;
        }
        return *this;
    }

    void UniformBuffer::Bind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, renderer_id_));
        Logger::GetLogger()->debug("Bound UniformBuffer (ID={}) to binding point={}.", renderer_id_, binding_point_);
    }

    void UniformBuffer::Unbind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, 0));
        Logger::GetLogger()->debug("Unbound UniformBuffer from binding point={}.", binding_point_);
    }

    void UniformBuffer::SetData(const void* data, GLsizeiptr size, GLintptr offset) const {
        // Check alignment for std140 layout (typically requires multiples of 16 bytes).
        if ((offset % 16 != 0) || (size % 16 != 0)) {
            Logger::GetLogger()->error("UniformBuffer::SetData alignment error: offset={} and size={} must be multiples of 16.", offset, size);
            throw std::runtime_error("UniformBuffer::SetData alignment error");
        }
        if (offset + size > size_) {
            Logger::GetLogger()->error("UniformBuffer::SetData out of range: offset({}) + size({}) > buffer size({}).", offset, size, size_);
            throw std::out_of_range("UniformBuffer::SetData out of range");
        }
        GLCall(glNamedBufferSubData(renderer_id_, offset, size, data));
        Logger::GetLogger()->debug("Updated UniformBuffer (ID={}) offset={} size={}.", renderer_id_, offset, size);
    }

    void* UniformBuffer::MapBuffer(GLenum access) const {
        void* ptr = glMapNamedBuffer(renderer_id_, access);
        if (!ptr) {
            Logger::GetLogger()->error("Failed to map UniformBuffer (ID={}).", renderer_id_);
        }
        return ptr;
    }

    void UniformBuffer::UnmapBuffer() const {
        GLCall(glUnmapNamedBuffer(renderer_id_));
        Logger::GetLogger()->debug("Unmapped UniformBuffer (ID={}).", renderer_id_);
    }

} // namespace Graphics
