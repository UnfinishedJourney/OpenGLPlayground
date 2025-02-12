#pragma once

#include <glad/glad.h>
#include <stdexcept>
#include <span>

namespace Graphics {

    /**
     * @brief Manages an OpenGL Shader Storage Buffer (SSBO).
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

        // Non-copyable
        ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

        // Movable
        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

        /**
         * @brief Binds the SSBO to GL_SHADER_STORAGE_BUFFER.
         */
        void Bind() const;

        void Unbind() const;

        /**
         * @brief Updates a region of the buffer with new data.
         * @param data A span of raw bytes representing the new data.
         * @param offset Byte offset into the buffer.
         * @throws std::runtime_error if offset + data size exceeds the buffer capacity.
         */
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        /**
         * @brief Binds the SSBO to its specified binding point.
         */
        void BindBase() const;

        [[nodiscard]] GLsizeiptr GetSize()         const { return size_; }
        [[nodiscard]] GLuint     GetRendererID()   const { return rendererID_; }
        [[nodiscard]] GLuint     GetBindingPoint() const { return bindingPoint_; }

    private:
        GLuint rendererID_{ 0 };
        GLuint bindingPoint_{ 0 };
        GLsizeiptr size_{ 0 };
        GLenum usage_{ GL_DYNAMIC_DRAW };
    };

} // namespace Graphics