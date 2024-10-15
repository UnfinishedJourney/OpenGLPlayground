#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GLContext 
{
public:
    static GLFWwindow* InitOpenGL(int width, int height, const std::string& title,
        GLFWkeyfun keyfun, GLFWscrollfun scrollfun, GLFWcursorposfun cursorposfun,
        GLFWwindowsizefun windowsizefun);
    static void Cleanup(GLFWwindow* window);
};