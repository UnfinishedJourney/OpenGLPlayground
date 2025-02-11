#pragma once
#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

namespace Graphics {

    /**
     * @brief Encapsulates an OpenGL Index Buffer Object (IBO).
     *
     * Stores indices for element drawing.
     */
    class IndexBuffer {
    public:
        /**
         * @brief Constructs an IndexBuffer and uploads the index data.
         * @param data  Span of indices.
         * @param usage OpenGL usage flag (e.g., GL_STATIC_DRAW).
         * @throws std::runtime_error if the buffer cannot be created.
         */
        IndexBuffer(std::span<const GLuint> data, GLenum usage = GL_STATIC_DRAW);

        ~IndexBuffer();

        // Non-copyable
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        // Movable
        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        /**
         * @brief Binds this IBO to GL_ELEMENT_ARRAY_BUFFER.
         */
        void Bind() const;

        void Unbind() const;

        /**
         * @brief Updates a subregion of the index buffer.
         * @param data   New index data.
         * @param offset Byte offset for the update.
         * @throws std::runtime_error if the update exceeds the buffer size.
         */
        void UpdateData(std::span<const GLuint> data, GLintptr offset = 0);

        [[nodiscard]] GLuint GetRendererID() const { return renderer_id_; }
        [[nodiscard]] GLsizei GetCount() const { return count_; }
        [[nodiscard]] size_t GetSize() const { return size_; }

    private:
        GLuint renderer_id_{ 0 };           ///< OpenGL buffer handle.
        GLsizei count_{ 0 };                ///< Number of indices.
        size_t size_{ 0 };                  ///< Buffer size in bytes.
        GLenum usage_{ GL_STATIC_DRAW };
    };

} // namespace Graphics