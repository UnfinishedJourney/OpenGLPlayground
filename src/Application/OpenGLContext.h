#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GLContext
{
public:
    static GLFWwindow* InitOpenGL(int width, int height, const std::string& title,
        GLFWkeyfun keyfun = nullptr,
        GLFWscrollfun scrollfun = nullptr,
        GLFWcursorposfun cursorposfun = nullptr,
        GLFWwindowsizefun windowsizefun = nullptr);
    static void Cleanup(GLFWwindow* window);

private:
    static void InitOpenGLDebug();
    static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam);
};