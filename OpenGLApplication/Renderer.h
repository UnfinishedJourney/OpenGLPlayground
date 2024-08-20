#pragma once
#include <GL/glew.h>

// Macro for error checking
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

// Functions for clearing and logging OpenGL errors
void GLClearError();

bool GlLogCall(const char* function, const char* file, int line);

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer
{
public:
    void Draw(VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
    void Clear() const;
};