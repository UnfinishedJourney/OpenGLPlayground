#pragma once

#include <glad/glad.h>
#include <stdexcept>
#include <span>

namespace graphics {

    /**
     * @brief Specifies the memory layout for uniform buffers.
     */
    enum class UniformBufferLayout {
        Std140, // Enforces 16-byte alignment
        Std430  // (Placeholder for different rules if needed)
    };

    /**
     * @brief Manages an OpenGL Uniform Buffer Object (UBO).
     */
    class UniformBuffer {
    public:
        /**
         * @brief Constructs a UniformBuffer with the given size and binding point.
         * @param size The size of the buffer in bytes.
         * @param bindingPoint The binding point index.
         * @param layout The memory layout (Std140 enforces 16-byte alignment).
         * @param usage Usage pattern (e.g., GL_DYNAMIC_DRAW).
         * @throws std::runtime_error if buffer creation fails.
         */
        explicit UniformBuffer(GLsizeiptr size, GLuint bindingPoint,
            UniformBufferLayout layout = UniformBufferLayout::Std140,
            GLenum usage = GL_DYNAMIC_DRAW);

        ~UniformBuffer();

        // Non-copyable
        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        // Movable
        UniformBuffer(UniformBuffer&& other) noexcept;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept;

        /**
         * @brief Binds the UBO to its binding point.
         */
        void Bind() const;

        void Unbind() const;

        /**
         * @brief Updates a subrange of the buffer with new data.
         * @param data A span of raw bytes representing the data.
         * @param offset Byte offset into the buffer.
         * @throws std::runtime_error if alignment or range check fails.
         */
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0) const;

        /**
         * @brief Maps the buffer for direct access.
         * @param access Access flags (e.g., GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE).
         * @return Pointer to the mapped memory.
         */
        void* MapBuffer(GLenum access) const;

        /**
         * @brief Unmaps the buffer.
         */
        void UnmapBuffer() const;

        [[nodiscard]] GLuint     GetRendererID()   const { return m_RendererID; }
        [[nodiscard]] GLuint     GetBindingPoint() const { return m_BindingPoint; }
        [[nodiscard]] GLsizeiptr GetSize()         const { return m_Size; }
        [[nodiscard]] UniformBufferLayout GetLayout() const { return m_Layout; }

    private:
        GLuint m_RendererID{ 0 };
        GLuint m_BindingPoint{ 0 };
        GLenum m_Usage{ GL_DYNAMIC_DRAW };
        GLsizeiptr m_Size{ 0 };
        UniformBufferLayout m_Layout{ UniformBufferLayout::Std140 };
    };

} // namespace graphics