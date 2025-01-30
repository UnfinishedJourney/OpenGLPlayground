#include "Application/OpenGLContext.h"

// Standard library
#include <iostream>

// Engine includes
#include "Utilities/Logger.h"

GLFWwindow* GLContext::InitOpenGL(int width, int height, const std::string& title)
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing GLFW...");

    if (!glfwInit())
    {
        logger->error("Failed to initialize GLFW.");
        return nullptr;
    }

    logger->info("GLFW initialized successfully.");

    // Configure GLFW to create a 4.6 core profile context with debug mode enabled
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        logger->error("Failed to create GLFW window.");
        glfwTerminate();
        return nullptr;
    }

    logger->info("GLFW window created ({}x{}): '{}'.", width, height, title);

    // Make the context current
    glfwMakeContextCurrent(window);

    // Load OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger->error("Failed to initialize GLAD.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
    logger->info("GLAD initialized successfully.");

    // Set the initial viewport
    glViewport(0, 0, width, height);

    // VSync off (for potentially better performance, but more CPU/GPU usage)
    glfwSwapInterval(0);

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Set up debug output
    InitOpenGLDebug();
    logger->info("OpenGL debug context initialized.");

    logger->info("OpenGL context is ready to use.");
    return window;
}

void GLContext::Cleanup(GLFWwindow* window)
{
    auto logger = Logger::GetLogger();
    logger->info("Cleaning up OpenGL context...");

    if (window)
    {
        glfwDestroyWindow(window);
        logger->debug("Destroyed GLFW window.");
    }

    glfwTerminate();
    logger->info("GLFW terminated. Cleanup complete.");
}

void GLAPIENTRY GLContext::OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei /*length*/,
    const GLchar* message, const void* /*userParam*/)
{
    auto logger = Logger::GetLogger();

    // Convert enums to strings for more readable logging
    std::string sourceStr;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    default:                              sourceStr = "Unknown"; break;
    }

    std::string typeStr;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    default:                                typeStr = "Unknown"; break;
    }

    std::string severityStr;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    default:                             severityStr = "Unknown"; break;
    }

    // Decide how to log based on severity
    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        logger->error("OpenGL [Source: {}, Type: {}, Severity: {}] (ID: {}): {}",
            sourceStr, typeStr, severityStr, id, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        logger->warn("OpenGL [Source: {}, Type: {}, Severity: {}] (ID: {}): {}",
            sourceStr, typeStr, severityStr, id, message);
    }
    else
    {
        // Low or Notification -> log as debug
        logger->debug("OpenGL [Source: {}, Type: {}, Severity: {}] (ID: {}): {}",
            sourceStr, typeStr, severityStr, id, message);
    }
}

void GLContext::InitOpenGLDebug()
{
    // Enable synchronous debug output so that errors are triggered immediately
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);

    // We can filter out certain IDs or severities if desired:
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}