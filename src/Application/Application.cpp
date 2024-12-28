#include "Application.h"

#include "Application/OpenGLContext.h"
#include "Scene/Screen.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "AllTests.h"       // Where your Test* classes are declared
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"

//#include <imgui.h>
//#include <imgui_impl_glfw.h>
//#include <imgui_impl_opengl3.h>

#include <chrono>
#include <sstream>

Application::Application()
    : m_CameraController(m_InputManager)
{
    PROFILE_FUNCTION(Magenta); // Profiling the constructor

    // Initialize logger
    Logger::Init();
    m_Logger = Logger::GetLogger();
    m_Logger->info("Application constructor called.");
}

Application::~Application()
{
    PROFILE_FUNCTION(Magenta); // Profiling the destructor
    m_Logger->info("Application destructor called. Goodbye!");
}

void Application::Init()
{
    PROFILE_FUNCTION(Magenta); // Profile the Init function

    // Create an OpenGL window
    m_Window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application");
    if (!m_Window) {
        m_Logger->critical("Failed to initialize OpenGL context. Exiting.");
        return;
    }
    glfwSetWindowUserPointer(m_Window, this);

    // Register callbacks (GLFW, input, resize, etc.)
    RegisterCallbacks();

    // Optionally, show the mouse cursor or not
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize ImGui
    InitializeImGui();

    // Initialize Test Menu
    {
        PROFILE_BLOCK("Initialize TestMenu", Yellow);
        m_TestMenu = std::make_shared<TestMenu>(m_TestManager);

        // Register available Tests
        m_TestMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
        m_TestMenu->RegisterTest("ClearColor", []() { return std::make_shared<TestClearColor>(); });
        m_TestMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
        m_TestMenu->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });

        // Register TestMenuTest itself (which displays the menu)
        m_TestManager.RegisterTest("Test Menu", [this]() {
            return std::make_shared<TestMenuTest>(m_TestMenu);
            });
        m_TestManager.SwitchTest("Test Menu");
    }

    // Create GPU Queries
    InitializeGpuQueries();
}

void Application::Run()
{
    PROFILE_FUNCTION(Cyan); // Profile the Run function

    if (!m_Window) {
        m_Logger->error("No valid window to run application. Did Init() fail?");
        return;
    }

    m_LastFrameTime = glfwGetTime();

    // Main Loop
    while (!glfwWindowShouldClose(m_Window)) {
        UpdateAndRenderFrame();
    }

    // Cleanup ImGui, GPU Queries, and Window
    ShutdownImGui();
    ShutdownGpuQueries();
    GLContext::Cleanup(m_Window);
}

void Application::UpdateAndRenderFrame()
{
    PROFILE_BLOCK("Frame", Purple);

    // Start CPU timer
    const auto cpuStart = std::chrono::high_resolution_clock::now();

    // Start GPU timer
    GLCall(glBeginQuery(GL_TIME_ELAPSED, m_GpuQueryStart));

    // Clear the framebuffer
    {
        PROFILE_BLOCK("Render Setup", Blue);
        GLCall(glClearColor(0.3f, 0.2f, 0.8f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    // Calculate deltaTime
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;

    // Handle input
    {
        PROFILE_BLOCK("Handle Input", Blue);
        glfwPollEvents();
        HandleInput(deltaTime);
    }

    // Update and render current test
    {
        PROFILE_BLOCK("Update and Render Test", Blue);
        m_TestManager.UpdateCurrentTest(static_cast<float>(deltaTime));
        m_TestManager.RenderCurrentTest();
        SynchronizeCameraController();
    }

    // Render ImGui
    {
        PROFILE_BLOCK("ImGui Rendering", Blue);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the ImGui content from current test
        m_TestManager.RenderCurrentTestImGui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // End GPU timer
    GLCall(glEndQuery(GL_TIME_ELAPSED));

    // (Optional) You might not always want to call glFinish here if you’re collecting pipeline stats.
    GLCall(glFinish());

    // Fetch GPU timing
    GLuint64 gpuTime = 0;
    GLCall(glGetQueryObjectui64v(m_GpuQueryStart, GL_QUERY_RESULT, &gpuTime));
    double gpuFrameTimeMs = static_cast<double>(gpuTime) / 1'000'000.0; // ns to ms

    // Swap buffers & update input
    {
        PROFILE_BLOCK("Swap Buffers", Blue);
        glfwSwapBuffers(m_Window);
        m_InputManager.Update();
    }

    // Stop CPU timer
    const auto cpuEnd = std::chrono::high_resolution_clock::now();
    const auto cpuFrameTimeMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();

    // Update window title with FPS / times
    ShowFpsAndFrameTimes(cpuFrameTimeMs, gpuFrameTimeMs);
}

void Application::HandleInput(double deltaTime)
{
    PROFILE_FUNCTION(Yellow);

    m_CameraController.Update(deltaTime);

    // Example: press R to reload shaders
    if (m_InputManager.WasKeyJustPressed(GLFW_KEY_R)) {
        m_Logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

void Application::SynchronizeCameraController()
{
    PROFILE_FUNCTION(Yellow);

    auto camera = m_TestManager.GetCurrentCamera();
    m_CameraController.SetCamera(camera);

    if (camera) {
        m_Logger->debug("CameraController synchronized with current test camera.");
    }
    else {
        m_Logger->debug("Current test does not have a camera.");
    }
}

void Application::ShowFpsAndFrameTimes(double cpuFrameTimeMs, double gpuFrameTimeMs)
{
    static double previousSeconds = 0.0;
    static int frameCount = 0;

    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - previousSeconds;
    frameCount++;

    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = static_cast<double>(frameCount) / elapsedSeconds;
        frameCount = 0;

        std::ostringstream titleStream;
        titleStream.precision(3);
        titleStream << std::fixed
            << "FPS: " << fps << "    "
            << "CPU: " << cpuFrameTimeMs << " ms    "
            << "GPU: " << gpuFrameTimeMs << " ms";

        glfwSetWindowTitle(m_Window, titleStream.str().c_str());
    }
}

void Application::RegisterCallbacks()
{
    PROFILE_FUNCTION(Green);

    // Key Callback
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        PROFILE_BLOCK("Key Callback Execution", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        app->m_InputManager.SetKey(key, isPressed);
        });

    // Cursor Position Callback
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        PROFILE_BLOCK("Cursor Position Callback Execution", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->m_CameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            app->m_CameraController.Reset();
        }
        });

    // Scroll Callback
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        PROFILE_BLOCK("Scroll Callback Execution", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->m_CameraController.ProcessMouseScroll(static_cast<float>(yoffset));
        });

    // Window Size Callback
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        PROFILE_BLOCK("Window Size Callback Execution", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        if (height == 0) {
            height = 1;
        }
        glViewport(0, 0, width, height);
        Screen::SetResolution(width, height);

        app->m_TestManager.HandleWindowResize(width, height);
        app->m_CameraController.UpdateFOV();
        });
}

void Application::InitializeImGui()
{
    PROFILE_BLOCK("Initialize ImGui", Yellow);

    m_Logger->info("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    const char* glsl_version = "#version 460";
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    m_Logger->info("ImGui initialized successfully.");
}

void Application::ShutdownImGui()
{
    PROFILE_BLOCK("Shutdown ImGui", Magenta);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::InitializeGpuQueries()
{
    PROFILE_BLOCK("Create GPU Queries", Yellow);

    GLCall(glGenQueries(1, &m_GpuQueryStart));
    GLCall(glGenQueries(1, &m_GpuQueryEnd));
    // You could use both start and end if you measure intervals or multiple passes.
}

void Application::ShutdownGpuQueries()
{
    PROFILE_BLOCK("Delete GPU Queries", Magenta);

    if (m_GpuQueryStart != 0) {
        GLCall(glDeleteQueries(1, &m_GpuQueryStart));
        m_GpuQueryStart = 0;
    }
    if (m_GpuQueryEnd != 0) {
        GLCall(glDeleteQueries(1, &m_GpuQueryEnd));
        m_GpuQueryEnd = 0;
    }
}