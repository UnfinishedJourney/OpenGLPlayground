#pragma once
#include <GL/glew.h>

#include <Utility.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Screen {
public:
    static int s_Width;
    static int s_Height;

    static void setResolution(int w, int h) {
        s_Width = w;
        s_Height = h;
    }
};

struct FrameData
{
    static glm::mat4 s_View;
    static glm::mat4 s_Projection;
};

class Renderer
{
public:
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
    void Clear() const;
};
