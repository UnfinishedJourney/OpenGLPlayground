#include "IndirectBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace graphics {

    IndirectBuffer::IndirectBuffer(std::span<const std::byte> data, GLenum usage)
        : size_(data.size_bytes())
    {
        GLCall(glCreateBuffers(1, &rendererId_));
        if (rendererId_ == 0) {
            throw std::runtime_error("Failed to create Indirect Buffer Object.");
        }
        GLCall(glNamedBufferData(rendererId_, size_, data.data(), usage));
        Logger::GetLogger()->info("Created IndirectBuffer with ID={} Size={} bytes.", rendererId_, size_);
    }

    IndirectBuffer::~IndirectBuffer() {
        if (rendererId_ != 0) {
            GLCall(glDeleteBuffers(1, &rendererId_));
            Logger::GetLogger()->info("Deleted IndirectBuffer with ID={}.", rendererId_);
        }
    }

    IndirectBuffer::IndirectBuffer(IndirectBuffer&& other) noexcept
        : rendererId_(other.rendererId_), size_(other.size_)
    {
        other.rendererId_ = 0;
        other.size_ = 0;
    }

    IndirectBuffer& IndirectBuffer::operator=(IndirectBuffer&& other) noexcept {
        if (this != &other) {
            if (rendererId_ != 0) {
                GLCall(glDeleteBuffers(1, &rendererId_));
            }
            rendererId_ = other.rendererId_;
            size_ = other.size_;
            other.rendererId_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void IndirectBuffer::Bind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, rendererId_));
    }

    void IndirectBuffer::Unbind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
    }

    void IndirectBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) {
        if (offset < 0 || static_cast<size_t>(offset) + data.size_bytes() > size_) {
            throw std::runtime_error("IndirectBuffer::UpdateData: Data update exceeds buffer size.");
        }
        GLCall(glNamedBufferSubData(rendererId_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated IndirectBuffer ID={} Offset={} Size={} bytes.",
            rendererId_, offset, data.size_bytes());
    }

} // namespace graphics