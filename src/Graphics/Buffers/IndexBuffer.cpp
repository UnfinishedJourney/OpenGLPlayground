#include "IndexBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace graphics {

    IndexBuffer::IndexBuffer(std::span<const GLuint> data, GLenum usage)
        : count_(static_cast<GLsizei>(data.size())),
        size_(data.size_bytes()),
        usage_(usage)
    {
        GLCall(glCreateBuffers(1, &rendererId_));
        if (rendererId_ == 0) {
            throw std::runtime_error("Failed to create Index Buffer Object.");
        }
        GLCall(glNamedBufferData(rendererId_, size_, data.data(), usage_));
        Logger::GetLogger()->info("Created IndexBuffer (ID={}) with Count={}.", rendererId_, count_);
    }

    IndexBuffer::~IndexBuffer() {
        if (rendererId_ != 0) {
            GLCall(glDeleteBuffers(1, &rendererId_));
            Logger::GetLogger()->info("Deleted IndexBuffer (ID={}).", rendererId_);
        }
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : rendererId_(other.rendererId_),
        count_(other.count_),
        size_(other.size_),
        usage_(other.usage_)
    {
        other.rendererId_ = 0;
        other.count_ = 0;
        other.size_ = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            if (rendererId_ != 0) {
                GLCall(glDeleteBuffers(1, &rendererId_));
            }
            rendererId_ = other.rendererId_;
            count_ = other.count_;
            size_ = other.size_;
            usage_ = other.usage_;
            other.rendererId_ = 0;
            other.count_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void IndexBuffer::Bind() const {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId_));
    }

    void IndexBuffer::Unbind() const {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void IndexBuffer::UpdateData(std::span<const GLuint> data, GLintptr offset) {
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(size_)) {
            throw std::runtime_error("IndexBuffer::UpdateData: Data exceeds buffer size.");
        }
        GLCall(glNamedBufferSubData(rendererId_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated IndexBuffer (ID={}) at offset={} with new data.", rendererId_, offset);
    }

} // namespace graphics