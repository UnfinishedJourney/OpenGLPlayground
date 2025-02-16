#include "Screen.h"

int Screen::width_ = 800;
int Screen::height_ = 600;
float Screen::displayHeight_ = 13.23f;
float Screen::viewerDistance_ = 24.0f;

void Screen::SetResolution(int width, int height) {
    width_ = width;
    height_ = height;
}