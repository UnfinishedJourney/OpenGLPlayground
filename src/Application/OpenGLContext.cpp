#include "Application/OpenGLContext.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void GLAPIENTRY GLContext::OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    auto logger = Logger::GetLogger();

    std::string sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    }

    std::string typeStr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }

    std::string severityStr;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    if (severity == GL_DEBUG_SEVERITY_HIGH)
        logger->error("OpenGL [{} - {} - {}] (ID: {}): {}", sourceStr, typeStr, severityStr, id, message);
}

void GLContext::InitOpenGLDebug() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
        0, nullptr, GL_TRUE); // Enable all messages
}

GLFWwindow* GLContext::InitOpenGL(int width, int height, const std::string& title,
    GLFWkeyfun keyfun,
    GLFWscrollfun scrollfun,
    GLFWcursorposfun cursorposfun,
    GLFWwindowsizefun windowsizefun)
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing GLFW...");

    if (!glfwInit()) {
        logger->error("Failed to initialize GLFW.");
        return nullptr;
    }

    logger->info("GLFW initialized successfully.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Enable OpenGL Debug Context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        logger->error("Failed to create GLFW window.");
        glfwTerminate();
        return nullptr;
    }

    logger->info("GLFW window created successfully. Size: {}x{}, Title: '{}'.", width, height, title);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logger->error("Failed to initialize GLAD.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    logger->info("GLAD initialized successfully.");

    glViewport(0, 0, width, height);

    // Set callbacks if provided
    if (keyfun) glfwSetKeyCallback(window, keyfun);
    if (scrollfun) glfwSetScrollCallback(window, scrollfun);
    if (cursorposfun) glfwSetCursorPosCallback(window, cursorposfun);
    if (windowsizefun) glfwSetWindowSizeCallback(window, windowsizefun);

    glfwSwapInterval(1); // Enable VSync

    glEnable(GL_DEPTH_TEST);

    logger->info("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    logger->info("ImGui initialized successfully.");

    // Initialize OpenGL Debugging
    InitOpenGLDebug();
    logger->info("OpenGL Debugging initialized.");

    logger->info("OpenGL context initialized and ready for use.");

    return window;
}

void GLContext::Cleanup(GLFWwindow* window)
{
    auto logger = Logger::GetLogger();
    logger->info("Starting cleanup of OpenGL context.");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    logger->debug("ImGui context shutdown successfully.");

    glfwDestroyWindow(window);
    logger->debug("Destroyed GLFW window.");

    glfwTerminate();
    logger->info("GLFW terminated. OpenGL context cleanup completed.");
}