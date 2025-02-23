#include "UniformBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace graphics {

    UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint,
        UniformBufferLayout layout, GLenum usage)
        : bindingPoint_(bindingPoint), usage_(usage), size_(size), layout_(layout)
    {
        GLCall(glCreateBuffers(1, &rendererId_));
        if (rendererId_ == 0) {
            Logger::GetLogger()->error("Failed to create Uniform Buffer Object.");
            throw std::runtime_error("Failed to create Uniform Buffer Object.");
        }
        GLCall(glNamedBufferData(rendererId_, size_, nullptr, usage_));
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, rendererId_));
        Logger::GetLogger()->info("Created UniformBuffer (ID={}) at binding point={} with size={} bytes.",
            rendererId_, bindingPoint_, size_);
    }

    UniformBuffer::~UniformBuffer() {
        if (rendererId_ != 0) {
            GLCall(glDeleteBuffers(1, &rendererId_));
            Logger::GetLogger()->info("Deleted UniformBuffer (ID={}).", rendererId_);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : rendererId_(other.rendererId_),
        bindingPoint_(other.bindingPoint_),
        usage_(other.usage_),
        size_(other.size_),
        layout_(other.layout_)
    {
        other.rendererId_ = 0;
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
        if (this != &other) {
            if (rendererId_ != 0) {
                GLCall(glDeleteBuffers(1, &rendererId_));
            }
            rendererId_ = other.rendererId_;
            bindingPoint_ = other.bindingPoint_;
            usage_ = other.usage_;
            size_ = other.size_;
            layout_ = other.layout_;
            other.rendererId_ = 0;
        }
        return *this;
    }

    void UniformBuffer::Bind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, rendererId_));
        Logger::GetLogger()->debug("Bound UniformBuffer (ID={}) to binding point={}.", rendererId_, bindingPoint_);
    }

    void UniformBuffer::Unbind() const {
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, 0));
        Logger::GetLogger()->debug("Unbound UniformBuffer from binding point={}.", bindingPoint_);
    }

    void UniformBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) const {
        if (layout_ == UniformBufferLayout::Std140) {
            if ((offset % 16 != 0) || (data.size_bytes() % 16 != 0)) {
                Logger::GetLogger()->error("UniformBuffer::UpdateData alignment error: offset={} and size={} must be multiples of 16 (Std140).", offset, data.size_bytes());
                throw std::runtime_error("UniformBuffer::UpdateData alignment error");
            }
        }
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(size_)) {
            Logger::GetLogger()->error("UniformBuffer::UpdateData out of range: offset({}) + size({}) > buffer size({}).", offset, data.size_bytes(), size_);
            throw std::out_of_range("UniformBuffer::UpdateData out of range");
        }
        GLCall(glNamedBufferSubData(rendererId_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated UniformBuffer (ID={}) offset={} size={} bytes.", rendererId_, offset, data.size_bytes());
    }

    void* UniformBuffer::MapBuffer(GLenum access) const {
        void* ptr = glMapNamedBuffer(rendererId_, access);
        if (!ptr) {
            Logger::GetLogger()->error("Failed to map UniformBuffer (ID={}).", rendererId_);
        }
        return ptr;
    }

    void UniformBuffer::UnmapBuffer() const {
        GLCall(glUnmapNamedBuffer(rendererId_));
        Logger::GetLogger()->debug("Unmapped UniformBuffer (ID={}).", rendererId_);
    }

} // namespace graphics