#pragma once

class Screen {
public:
    // Resolution and physical display parameters.
    static int width_;
    static int height_;
    static float displayHeight_;   // e.g. inches
    static float viewerDistance_;  // same units as display height

    static void SetResolution(int width, int height);
};