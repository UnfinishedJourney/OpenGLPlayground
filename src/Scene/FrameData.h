#pragma once

#include "glm.hpp"

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