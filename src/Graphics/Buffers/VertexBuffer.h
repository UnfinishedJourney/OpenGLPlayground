#pragma once

#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

namespace Graphics {

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
        VertexBuffer(std::span<const std::byte> data, GLenum usage = GL_STATIC_DRAW);

        ~VertexBuffer();

        // Non-copyable
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        // Movable
        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        /**
         * @brief Binds this VBO to the GL_ARRAY_BUFFER target.
         */
        void Bind() const;

        /**
         * @brief Unbinds any VBO from the GL_ARRAY_BUFFER target.
         */
        void Unbind() const;

        /**
         * @brief Updates a subregion of the buffer with new data.
         * @param data   A span of data to update.
         * @param offset Byte offset at which to start updating.
         * @throws std::runtime_error if the update exceeds the buffer size.
         */
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        [[nodiscard]] size_t GetSize() const { return m_Size; }
        [[nodiscard]] GLuint GetRendererID() const { return m_RendererID; }

    private:
        GLuint m_RendererID{ 0 };  ///< OpenGL buffer handle (0 if uninitialized)
        size_t m_Size{ 0 };        ///< Buffer size in bytes
        GLenum m_Usage{ GL_STATIC_DRAW };
    };

}