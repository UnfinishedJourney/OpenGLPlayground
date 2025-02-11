#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
        : count_(static_cast<GLsizei>(data.size())),
        size_(data.size_bytes()),
        usage_(usage)
    {
        GLCall(glCreateBuffers(1, &renderer_id_));
        if (renderer_id_ == 0) {
            throw std::runtime_error("Failed to create Index Buffer Object.");
        }
        GLCall(glNamedBufferData(renderer_id_, size_, data.data(), usage_));
        Logger::GetLogger()->info("Created IndexBuffer (ID={}) with Count={}.", renderer_id_, count_);
    }

    IndexBuffer::~IndexBuffer()
    {
        if (renderer_id_ != 0) {
            GLCall(glDeleteBuffers(1, &renderer_id_));
            Logger::GetLogger()->info("Deleted IndexBuffer (ID={}).", renderer_id_);
        }
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : renderer_id_(other.renderer_id_),
        count_(other.count_),
        size_(other.size_),
        usage_(other.usage_)
    {
        other.renderer_id_ = 0;
        other.count_ = 0;
        other.size_ = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
    {
        if (this != &other) {
            if (renderer_id_ != 0) {
                GLCall(glDeleteBuffers(1, &renderer_id_));
            }
            renderer_id_ = other.renderer_id_;
            count_ = other.count_;
            size_ = other.size_;
            usage_ = other.usage_;
            other.renderer_id_ = 0;
            other.count_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void IndexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_));
    }

    void IndexBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void IndexBuffer::UpdateData(std::span<const GLuint> data, GLintptr offset)
    {
        // Ensure that the update does not exceed the total buffer size.
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(size_)) {
            throw std::runtime_error("IndexBuffer::UpdateData: Data exceeds buffer size.");
        }

        // Update the specified subregion of the buffer.
        GLCall(glNamedBufferSubData(renderer_id_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated IndexBuffer (ID={}) at offset={} with new data.", renderer_id_, offset);
    }

} // namespace Graphics