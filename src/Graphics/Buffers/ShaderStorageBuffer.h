#pragma once

#include <glad/glad.h>
#include <stdexcept>
#include <span>

//maybe it should be inside textures
namespace graphics {

    /**
     * @brief Manages an OpenGL Shader Storage Buffer (SSBO).
     */
    class ShaderStorageBuffer {
    public:
        ShaderStorageBuffer(GLuint bindingPoint, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);
        ~ShaderStorageBuffer();

        // Non-copyable.
        ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;
        // Movable.
        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

        void Bind() const;
        void Unbind() const;
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        [[nodiscard]] GLsizeiptr GetSize()         const { return size_; }
        [[nodiscard]] GLuint     GetRendererID()   const { return rendererId_; }
        [[nodiscard]] GLuint     GetBindingPoint() const { return bindingPoint_; }

    private:
        GLuint rendererId_{ 0 };
        GLuint bindingPoint_{ 0 };
        GLsizeiptr size_{ 0 };
        GLenum usage_{ GL_DYNAMIC_DRAW };
    };

} // namespace graphics