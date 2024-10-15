#include "Application/OpenGLContext.h"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Utilities/Logger.h"

void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_HIGH) 
    {
        Logger::GetLogger()->error("[OpenGL Error] (ID: {}) {}", id, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) 
    {
        Logger::GetLogger()->warn("[OpenGL Warning] (ID: {}) {}", id, message);
    }
    else 
    {
        Logger::GetLogger()->info("[OpenGL Info] (ID: {}) {}", id, message);
    }
}

void InitOpenGLDebug() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
        0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH,
        0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM,
        0, nullptr, GL_TRUE);
}

GLFWwindow* GLContext::InitOpenGL(int width, int height, const std::string& title,
    GLFWkeyfun keyfun, GLFWscrollfun scrollfun, GLFWcursorposfun cursorposfun,
    GLFWwindowsizefun windowsizefun)
{
    Logger::GetLogger()->info("Initializing GLFW...");

    if (!glfwInit()) 
    {
        Logger::GetLogger()->error("Failed to initialize GLFW.");
        exit(EXIT_FAILURE);
    }
    Logger::GetLogger()->info("GLFW initialized successfully.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    Logger::GetLogger()->debug("Set GLFW window hints: OpenGL 4.5 Core Profile.");

    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) 
    {
        Logger::GetLogger()->error("Failed to create GLFW window.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    Logger::GetLogger()->info("GLFW window created successfully. Size: {}x{}, Title: '{}'.", width, height, title);

    glfwMakeContextCurrent(window);
    Logger::GetLogger()->debug("Made the created GLFW window the current context.");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        Logger::GetLogger()->error("Failed to initialize GLAD.");
        exit(EXIT_FAILURE);
    }
    Logger::GetLogger()->info("GLAD initialized successfully.");

    glViewport(0, 0, width, height);
    Logger::GetLogger()->debug("Set viewport to (0, 0, {}, {}).", width, height);

    glfwSetKeyCallback(window, keyfun);
    glfwSetScrollCallback(window, scrollfun);
    glfwSetCursorPosCallback(window, cursorposfun);
    glfwSetWindowSizeCallback(window, windowsizefun);
    Logger::GetLogger()->debug("Set GLFW callbacks: Key, Scroll, CursorPos, WindowSize.");

    glfwSwapInterval(1);
    Logger::GetLogger()->debug("Enabled VSync (Swap Interval 1).");

    glEnable(GL_DEPTH_TEST);
    Logger::GetLogger()->debug("Enabled GL_DEPTH_TEST.");

    Logger::GetLogger()->info("Initializing ImGui...");
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    Logger::GetLogger()->info("ImGui initialized successfully.");

    InitOpenGLDebug();
    Logger::GetLogger()->info("OpenGL Debugger initialized.");

    Logger::GetLogger()->info("OpenGL context initialized and ready for use.");

    return window;
}

void GLContext::Cleanup(GLFWwindow* window)
{
    Logger::GetLogger()->info("Starting cleanup of OpenGL context.");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Logger::GetLogger()->debug("ImGui context shutdown successfully.");

    glfwDestroyWindow(window);
    Logger::GetLogger()->debug("Destroyed GLFW window.");

    glfwTerminate();
    Logger::GetLogger()->info("GLFW terminated. OpenGL context cleanup completed.");
}