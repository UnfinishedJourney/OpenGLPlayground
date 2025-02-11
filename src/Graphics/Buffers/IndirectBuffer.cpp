#include "IndirectBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>

namespace Graphics {

    IndirectBuffer::IndirectBuffer(std::span<const std::byte> data, GLenum usage)
        : size_(data.size_bytes())
    {
        GLCall(glCreateBuffers(1, &renderer_id_));
        if (renderer_id_ == 0) {
            throw std::runtime_error("Failed to create Indirect Buffer Object.");
        }

        GLCall(glNamedBufferData(renderer_id_, size_, data.data(), usage));
        Logger::GetLogger()->info("Created IndirectBuffer with ID={} Size={} bytes.", renderer_id_, size_);
    }

    IndirectBuffer::IndirectBuffer() {
        GLCall(glCreateBuffers(1, &renderer_id_));
        if (renderer_id_ == 0) {
            throw std::runtime_error("Failed to create Indirect Buffer Object.");
        }
        size_ = 0;
    }

    IndirectBuffer::~IndirectBuffer() {
        if (renderer_id_ != 0) {
            GLCall(glDeleteBuffers(1, &renderer_id_));
            Logger::GetLogger()->info("Deleted IndirectBuffer with ID={}.", renderer_id_);
        }
    }

    IndirectBuffer::IndirectBuffer(IndirectBuffer&& other) noexcept
        : renderer_id_(other.renderer_id_), size_(other.size_)
    {
        other.renderer_id_ = 0;
        other.size_ = 0;
    }

    IndirectBuffer& IndirectBuffer::operator=(IndirectBuffer&& other) noexcept {
        if (this != &other) {
            if (renderer_id_ != 0) {
                GLCall(glDeleteBuffers(1, &renderer_id_));
            }
            renderer_id_ = other.renderer_id_;
            size_ = other.size_;
            other.renderer_id_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void IndirectBuffer::Bind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer_id_));
    }

    void IndirectBuffer::Unbind() const {
        GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
    }

    void IndirectBuffer::UpdateData(std::span<const std::byte> data, GLintptr offset) {
        if (offset < 0 || static_cast<size_t>(offset) + data.size_bytes() > size_) {
            throw std::runtime_error("IndirectBuffer::UpdateData: Data update exceeds buffer size.");
        }

        GLCall(glNamedBufferSubData(renderer_id_, offset, data.size_bytes(), data.data()));
        Logger::GetLogger()->debug("Updated IndirectBuffer ID={} Offset={} Size={}.", renderer_id_, offset, data.size_bytes());
    }

    void IndirectBuffer::SetData(std::span<const std::byte> data, GLenum usage) {
        // Reallocate and upload new data using DSA. (don't need to delete the buffer)
        GLCall(glNamedBufferData(renderer_id_, data.size_bytes(), data.data(), usage));
        size_ = data.size_bytes();
        Logger::GetLogger()->info("Set IndirectBuffer ID={} new data. Size={}.", renderer_id_, size_);
    }

} // namespace Graphics