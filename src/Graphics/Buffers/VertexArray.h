#pragma once

#include <vector>
#include <glad/glad.h>
#include <stdexcept>

namespace Graphics {

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
         * @param vertex_buffer  The source VertexBuffer.
         * @param layout         The layout describing the buffer’s vertex attributes.
         * @param binding_index  The binding index within the VAO (must be unique).
         * @throws std::invalid_argument if the binding index is already in use.
         */
        void AddBuffer(const VertexBuffer& vertex_buffer,
            const VertexBufferLayout& layout,
            GLuint binding_index = 0);

        /**
         * @brief Associates an IndexBuffer (element buffer) with this VAO.
         */
        void SetIndexBuffer(const IndexBuffer& index_buffer);

        /**
         * @brief Binds the VAO.
         */
        void Bind() const;

        /**
         * @brief Unbinds any VAO.
         */
        void Unbind() const;

        [[nodiscard]] GLuint GetRendererID() const { return renderer_id_; }

    private:
        GLuint renderer_id_{ 0 };               ///< OpenGL VAO handle
        std::vector<GLuint> binding_indices_;   ///< Tracks used binding indices to avoid duplicates
    };

} // namespace Graphics