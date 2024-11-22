#pragma once

#include <glad/glad.h>

struct BufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteBuffers(1, id);
        }
        delete id;
    }
};

struct VertexArrayDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteVertexArrays(1, id);
        }
        delete id;
    }
};

struct FrameBufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteFramebuffers(1, id);
        }
        delete id;
    }
};

struct TextureDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteTextures(1, id);
        }
        delete id;
    }
};

struct RenderBufferDeleter {
    void operator()(GLuint* id) const {
        if (id && *id != 0) {
            glDeleteRenderbuffers(1, id);
        }
        delete id;
    }
};
