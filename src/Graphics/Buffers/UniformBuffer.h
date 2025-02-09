#pragma once

#include <glad/glad.h>
#include <stdexcept>

namespace Graphics {

    /**
     * @brief Manages an OpenGL Uniform Buffer Object (UBO).
     *
     * Provides functions for binding, updating subdata, and mapping the buffer.
     */
    class UniformBuffer {
    public:
        /**
         * @brief Constructs a UniformBuffer with the given size and binding point.
         * @param size The size of the buffer in bytes.
         * @param bindingPoint The binding point index.
         * @param usage Usage pattern (e.g., GL_DYNAMIC_DRAW).
         * @throws std::runtime_error if buffer creation fails.
         */
        UniformBuffer(GLsizeiptr size, GLuint bindingPoint, GLenum usage = GL_DYNAMIC_DRAW);

        ~UniformBuffer();

        // Non-copyable; movable.
        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;
        UniformBuffer(UniformBuffer&& other) noexcept;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept;

        /**
         * @brief Binds the UBO to its binding point.
         */
        void Bind() const;

        /**
         * @brief Unbinds any UBO from the binding point.
         */
        void Unbind() const;

        /**
         * @brief Updates a subrange of the buffer with new data.
         * @param data Pointer to the data.
         * @param size Size in bytes of the data.
         * @param offset Byte offset into the buffer.
         * @throws std::runtime_error if alignment or range check fails.
         */
        void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

        /**
         * @brief Maps the buffer for direct access.
         * @param access Access flags (e.g., GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE).
         * @return Pointer to mapped memory.
         */
        void* MapBuffer(GLenum access) const;

        /**
         * @brief Unmaps the buffer.
         */
        void UnmapBuffer() const;

        [[nodiscard]] GLuint GetRendererID()   const { return m_RendererID; }
        [[nodiscard]] GLuint GetBindingPoint() const { return m_BindingPoint; }
        [[nodiscard]] GLsizeiptr GetSize()       const { return m_Size; }

    private:
        GLuint     m_BindingPoint = 0;
        GLenum     m_Usage = GL_DYNAMIC_DRAW;
        GLsizeiptr m_Size = 0;
        GLuint     m_RendererID = 0;
    };

} // namespace Graphics