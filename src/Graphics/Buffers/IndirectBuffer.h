#pragma once

#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

namespace graphics {

    /**
     * @brief Manages an OpenGL buffer for indirect draw commands.
     */
    class IndirectBuffer {
    public:
        IndirectBuffer(std::span<const std::byte> data, GLenum usage = GL_DYNAMIC_DRAW);
        ~IndirectBuffer();

        // Non-copyable.
        IndirectBuffer(const IndirectBuffer&) = delete;
        IndirectBuffer& operator=(const IndirectBuffer&) = delete;
        // Movable.
        IndirectBuffer(IndirectBuffer&& other) noexcept;
        IndirectBuffer& operator=(IndirectBuffer&& other) noexcept;

        void Bind() const;
        void Unbind() const;
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        [[nodiscard]] GLuint GetRendererID() const { return rendererId_; }
        [[nodiscard]] size_t GetBufferSize() const { return size_; }

    private:
        GLuint rendererId_{ 0 };
        size_t size_{ 0 };
    };

} // namespace graphics