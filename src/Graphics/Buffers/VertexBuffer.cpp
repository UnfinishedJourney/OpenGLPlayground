#include "VertexBuffer.h"
#include "Utilities/Logger.h"   
#include "Utilities/Utility.h"  // Contains GLCall macro for error checking
#include <stdexcept>

namespace graphics {

    VertexBuffer::VertexBuffer(std::span<const std::byte> data, GLenum usage)
        : size_{ data.size_bytes() }, usage_{ usage }
    {

        GLCall(glCreateBuffers(1, &rendererId_));
        if (rendererId_ == 0) {
            throw std::runtime_error("Failed to create Vertex Buffer Object.");
        }

        GLCall(glNamedBufferData(rendererId_, size_, data.data(), usage_));
        Logger::GetLogger()->info("Created VertexBuffer (ID={}) with size={} bytes.", rendererId_, size_);
    }

    VertexBuffer::~VertexBuffer()
    {
        if (rendererId_ != 0) {
            GLCall(glDeleteBuffers(1, &rendererId_));
            Logger::GetLogger()->info("Deleted VertexBuffer (ID={}).", rendererId_);
        }
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : rendererId_{ other.rendererId_ },
        size_{ other.size_ },
        usage_{ other.usage_ }
    {
        other.rendererId_ = 0;
        other.size_ = 0;
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
    {
        if (this != &other) {
            if (rendererId_ != 0) {
                GLCall(glDeleteBuffers(1, &rendererId_));
            }
            rendererId_ = other.rendererId_;
            size_ = other.size_;
            usage_ = other.usage_;
            other.rendererId_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void VertexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererId_));
    }

    void VertexBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    void VertexBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset)
    {
        if (offset + static_cast<GLintptr>(data.size_bytes()) > static_cast<GLintptr>(size_)) {
            throw std::runtime_error("VertexBuffer::UpdateData: Data exceeds buffer size.");
        }
        GLCall(glNamedBufferSubData(rendererId_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated VertexBuffer (ID={}) offset={} size={}.", rendererId_, offset, data.size_bytes());
    }

} // namespace graphics