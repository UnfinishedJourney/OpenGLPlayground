#pragma once
#include <glad/glad.h>
#include <span>
#include <cstddef>
#include <stdexcept>

namespace Graphics {

    //maybe should use mapping instead of glNamedBufferSubData and maybe should delete default constructor and setdata

    /**
     * @brief Manages an OpenGL buffer for indirect draw commands (GL_DRAW_INDIRECT_BUFFER).
     */
    class IndirectBuffer {
    public:
        /**
         * @brief Constructs and allocates the buffer with the provided data.
         * @param data Raw byte data to store in the buffer.
         * @param usage Typical usage pattern (e.g., GL_DYNAMIC_DRAW).
         * @throws std::runtime_error if the buffer cannot be created.
         */
        IndirectBuffer(std::span<const std::byte> data, GLenum usage = GL_DYNAMIC_DRAW);

        /**
         * @brief Default constructor. Call SetData later to allocate storage.
         */
        IndirectBuffer();

        ~IndirectBuffer();

        // Non-copyable
        IndirectBuffer(const IndirectBuffer&) = delete;
        IndirectBuffer& operator=(const IndirectBuffer&) = delete;

        // Movable
        IndirectBuffer(IndirectBuffer&& other) noexcept;
        IndirectBuffer& operator=(IndirectBuffer&& other) noexcept;

        /**
         * @brief Binds this buffer as GL_DRAW_INDIRECT_BUFFER.
         */
        void Bind() const;

        void Unbind() const;

        /**
         * @brief Updates a subregion of the buffer with new data.
         * @param data New data to update.
         * @param offset Byte offset in the buffer to update.
         */
        void UpdateData(std::span<const std::byte> data, GLintptr offset = 0);

        /**
         * @brief Allocates buffer storage and uploads new data.
         * @param data New data to store.
         * @param usage Usage hint (e.g., GL_DYNAMIC_DRAW).
         */
        void SetData(std::span<const std::byte> data, GLenum usage = GL_DYNAMIC_DRAW);

        [[nodiscard]] GLuint GetRendererID() const { return renderer_id_; }
        [[nodiscard]] size_t GetBufferSize() const { return size_; }

    private:
        GLuint renderer_id_{ 0 };   ///< OpenGL buffer handle.
        size_t size_ = 0;      ///< Current buffer size in bytes.
    };

} // namespace Graphics