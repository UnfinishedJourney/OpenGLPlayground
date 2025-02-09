#pragma once

#include <glad/glad.h>
#include <stdexcept>

namespace Graphics {

    /**
     * @brief Manages an OpenGL Shader Storage Buffer (SSBO).
     *
     * Allows binding at a specified binding point for use in shader programs.
     */
    class ShaderStorageBuffer {
    public:
        /**
         * @brief Constructs a ShaderStorageBuffer with the given binding point and size.
         * @param bindingPoint The binding point index.
         * @param size The total size in bytes.
         * @param usage Usage pattern (e.g., GL_DYNAMIC_DRAW).
         * @throws std::runtime_error if buffer creation fails.
         */
        ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);

        ~ShaderStorageBuffer();

        // Non-copyable; movable.
        ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

        /**
         * @brief Binds the SSBO to GL_SHADER_STORAGE_BUFFER.
         */
        void Bind() const;

        /**
         * @brief Unbinds the SSBO.
         */
        void Unbind() const;

        /**
         * @brief Updates a region of the buffer with new data.
         * @param data Pointer to the new data.
         * @param size Size in bytes of the data.
         * @param offset Byte offset into the buffer.
         * @throws std::runtime_error if size + offset exceed the buffer capacity.
         */
        void SetData(const void* data, GLsizeiptr size, GLintptr offset = 0);

        /**
         * @brief Binds the SSBO to its specified binding point.
         */
        void BindBase() const;

        [[nodiscard]] GLsizeiptr GetSize()         const { return m_Size; }
        [[nodiscard]] GLuint     GetRendererID()   const { return m_RendererID; }
        [[nodiscard]] GLuint     GetBindingPoint() const { return m_BindingPoint; }

    private:
        GLuint      m_BindingPoint = 0;
        GLsizeiptr  m_Size = 0;
        GLenum      m_Usage = GL_DYNAMIC_DRAW;
        GLuint      m_RendererID = 0;
    };

} // namespace Graphics