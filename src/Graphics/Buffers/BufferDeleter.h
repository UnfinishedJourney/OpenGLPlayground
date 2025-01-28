#pragma once

#include <glad/glad.h>

/**
 * @brief A functor for use with std::unique_ptr to delete OpenGL buffers.
 */
struct BufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteBuffers(1, id);
        }
        delete id;
    }
};

/**
 * @brief A functor for use with std::unique_ptr to delete OpenGL vertex arrays.
 */
struct VertexArrayDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteVertexArrays(1, id);
        }
        delete id;
    }
};

/**
 * @brief A functor for use with std::unique_ptr to delete OpenGL framebuffers.
 */
struct FrameBufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteFramebuffers(1, id);
        }
        delete id;
    }
};

/**
 * @brief A functor for use with std::unique_ptr to delete OpenGL textures.
 */
struct TextureDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteTextures(1, id);
        }
        delete id;
    }
};

/**
 * @brief A functor for use with std::unique_ptr to delete OpenGL renderbuffers.
 */
struct RenderBufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteRenderbuffers(1, id);
        }
        delete id;
    }
};