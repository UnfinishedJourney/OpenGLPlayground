#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * @brief Responsible for initializing and cleaning up the OpenGL/GLFW context.
 */
class GLContext
{
public:
    static GLFWwindow* InitOpenGL(int width, int height, const std::string& title);
    static void Cleanup(GLFWwindow* window);

private:
    static void InitOpenGLDebug();
    static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam);
};