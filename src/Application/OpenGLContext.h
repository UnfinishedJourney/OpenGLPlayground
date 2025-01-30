#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * @class GLContext
 * @brief Responsible for creating and cleaning up the OpenGL/GLFW context,
 *        and for configuring debug callbacks.
 */
class GLContext
{
public:
    /**
     * @brief Creates a GLFW window and initializes the OpenGL context (via glad).
     * @param width The initial width of the window.
     * @param height The initial height of the window.
     * @param title The window title string.
     * @return A pointer to the created GLFWwindow, or nullptr on failure.
     */
    static GLFWwindow* InitOpenGL(int width, int height, const std::string& title);

    /**
     * @brief Destroys the given GLFW window and terminates GLFW.
     * @param window A pointer to a valid GLFWwindow.
     */
    static void Cleanup(GLFWwindow* window);

private:
    /**
     * @brief Initializes OpenGL's debug message callback (if available).
     */
    static void InitOpenGLDebug();

    /**
     * @brief Callback function for OpenGL to report debug messages, warnings, or errors.
     */
    static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam);
};