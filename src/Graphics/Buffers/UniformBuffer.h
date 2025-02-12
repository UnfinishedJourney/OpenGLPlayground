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
         * @param binding_point The binding point index.
         * @param usage Usage pattern (e.g., GL_DYNAMIC_DRAW).
         * @throws std::runtime_error if buffer creation fails.
         */
        explicit UniformBuffer(GLsizeiptr size, GLuint binding_point, GLenum usage = GL_DYNAMIC_DRAW);

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
         *
         * Mapping a buffer means obtaining a pointer to its memory so that the CPU
         * can directly read from or write to it. This can be useful for dynamic data updates
         * or read-back of GPU results.
         *
         * @param access Access flags (e.g., GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE).
         * @return Pointer to the mapped memory.
         */
        void* MapBuffer(GLenum access) const;

        /**
         * @brief Unmaps the buffer.
         *
         * Unmapping releases the pointer obtained by MapBuffer. After unmapping,
         * the pointer is no longer valid, and the data is available for GPU operations.
         */
        void UnmapBuffer() const;

        [[nodiscard]] GLuint     GetRendererID()   const { return renderer_id_; }
        [[nodiscard]] GLuint     GetBindingPoint() const { return binding_point_; }
        [[nodiscard]] GLsizeiptr GetSize()       const { return size_; }

    private:
        GLuint     binding_point_ = 0;   ///< UBO binding point.
        GLenum     usage_ = GL_DYNAMIC_DRAW;
        GLsizeiptr size_ = 0;   ///< Buffer size in bytes.
        GLuint     renderer_id_ = 0;   ///< OpenGL buffer handle.
    };

} // namespace Graphics