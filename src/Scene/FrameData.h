#pragma once

#include "glm/glm.hpp"

class Screen {
public:
    static int s_Width;
    static int s_Height;

    static void SetResolution(int width, int height) {
        s_Width = width;
        s_Height = height;
    }
};

struct FrameData {
    static glm::mat4 s_View;
    static glm::mat4 s_Projection;
    static glm::vec3 s_CameraPos;
};