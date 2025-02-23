#pragma once

#include <vector>
#include <glad/glad.h>
#include <stdexcept>

namespace graphics {

    // Forward declarations
    class VertexBuffer;
    class IndexBuffer;
    class VertexBufferLayout;

    /**
     * @brief Encapsulates an OpenGL Vertex Array Object (VAO).
     *
     * Manages the association of vertex buffers and their attribute layouts.
     */
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        // Non-copyable
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        // Movable
        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        /**
         * @brief Attaches a vertex buffer and its layout to this VAO.
         *
         * @param vertexBuffer  The source VertexBuffer.
         * @param layout         The layout describing the buffer’s vertex attributes.
         * @param bindingIndex  The binding index within the VAO (must be unique).
         * @throws std::invalid_argument if the binding index is already in use.
         */
        void AddBuffer(const VertexBuffer& vertexBuffer,
            const VertexBufferLayout& layout,
            GLuint bindingIndex = 0);

        /**
         * @brief Associates an IndexBuffer (element buffer) with this VAO.
         */
        void SetIndexBuffer(const IndexBuffer& indexBuffer);

        /**
         * @brief Binds the VAO.
         */
        void Bind() const;

        /**
         * @brief Unbinds any VAO.
         */
        void Unbind() const;

        [[nodiscard]] GLuint GetRendererID() const { return rendererId_; }

    private:
        GLuint rendererId_{ 0 };               ///< OpenGL VAO handle.
        std::vector<GLuint> bindingIndices_;   ///< Tracks used binding indices to avoid duplicates.
    };

} // namespace graphics