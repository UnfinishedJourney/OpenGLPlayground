#pragma once

#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

namespace graphics {

    /**
     * @brief Encapsulates an OpenGL Index Buffer Object.
     */
    class IndexBuffer {
    public:
        IndexBuffer(std::span<const GLuint> data, GLenum usage = GL_STATIC_DRAW);
        ~IndexBuffer();

        // Non-copyable.
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;
        // Movable.
        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        void Bind() const;
        void Unbind() const;
        void UpdateData(std::span<const GLuint> data, GLintptr offset = 0);

        [[nodiscard]] GLuint GetRendererID() const { return rendererId_; }
        [[nodiscard]] GLsizei GetCount() const { return count_; }
        [[nodiscard]] size_t GetSize() const { return size_; }

    private:
        GLuint rendererId_{ 0 };
        GLsizei count_{ 0 };
        size_t size_{ 0 };
        GLenum usage_{ GL_STATIC_DRAW };
    };

} // namespace graphics