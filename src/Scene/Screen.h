#pragma once

#include "glm/glm.hpp"

class Screen {
public:
    static int s_Width;
    static int s_Height;

    // Physical display parameters
    static float s_DisplayHeight;      // in units (e.g., inches)
    static float s_ViewerDistance;     // in same units as display height

    // Method to set resolution and physical parameters
    static void SetResolution(int width, int height, float displayHeight, float viewerDistance) {
        s_Width = width;
        s_Height = height;
        s_DisplayHeight = displayHeight;
        s_ViewerDistance = viewerDistance;
    }
};