#pragma once

#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

//maybe need base GLBuffer class
namespace graphics {

    /**
     * @brief Encapsulates an OpenGL Vertex Buffer Object (VBO).
     *
     * Manages a GPU buffer storing vertex data.
     */
    class VertexBuffer {
    public:
        /**
         * @brief Constructs a VertexBuffer and uploads the provided data.
         * @param data  A span of raw bytes representing vertex data.
         * @param usage OpenGL usage flag (e.g., GL_STATIC_DRAW).
         * @throws std::runtime_error if the buffer cannot be created.
         */
        explicit VertexBuffer(std::span<const std::byte> data, GLenum usage = GL_STATIC_DRAW);

        ~VertexBuffer();

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        /**
         * @brief Binds this VBO to the GL_ARRAY_BUFFER target.
         */
        void Bind() const;

        void Unbind() const;

        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        [[nodiscard]] size_t GetSize() const { return size_; }
        [[nodiscard]] GLuint GetRendererID() const { return rendererId_; }

    private:
        GLuint rendererId_{ 0 };            ///< OpenGL buffer handle.
        size_t size_{ 0 };                  ///< Buffer size in bytes.
        GLenum usage_{ GL_STATIC_DRAW };
    };

} // namespace graphics