#include "Application.h"

#include "Application/OpenGLContext.h"
#include "Renderer/Renderer.h"            
#include "Scene/Screen.h"
#include "AllTests.h"                     
#include "Resources/ResourceManager.h"    
#include "Resources/ShaderManager.h"     
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"

#include <chrono>
#include <sstream>

Application::Application()
    : m_CameraController(m_InputManager)
{
    PROFILE_FUNCTION(Magenta); // Profile constructor

    Logger::Init(); // Initialize global logger
    m_Logger = Logger::GetLogger();
    m_Logger->info("Application constructor called.");
}

Application::~Application()
{
    PROFILE_FUNCTION(Magenta); // Profile destructor
    m_Logger->info("Application destructor called.");
}

void Application::Init()
{
    PROFILE_FUNCTION(Magenta); // Profile the Init function

    // Initialize the GLFW context + OpenGL context
    m_Window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application");
    if (!m_Window) {
        m_Logger->critical("Failed to initialize OpenGL context. Exiting.");
        return;
    }
    glfwSetWindowUserPointer(m_Window, this);

    RegisterCallbacks();
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize ImGui
    InitializeImGui();

    // Create the TestMenu and set up tests
    {
        PROFILE_BLOCK("Initialize TestMenu", Yellow);
        m_TestMenu = std::make_shared<TestMenu>(m_TestManager);

        // Register some tests
        m_TestMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>();      });
        m_TestMenu->RegisterTest("ClearColor", []() { return std::make_shared<TestClearColor>();  });
        m_TestMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>();      });
        m_TestMenu->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });
        m_TestMenu->RegisterTest("PBRHelmet", []() { return std::make_shared<TestDamagedHelmet>(); });

        // Register a special test that displays the menu itself
        m_TestManager.RegisterTest("Test Menu", [this]() {
            return std::make_shared<TestMenuTest>(m_TestMenu);
            });
        m_TestManager.SwitchTest("Test Menu");
    }

    // Initialize GPU queries
    //InitializeGpuQueries();
}

void Application::Run()
{
    PROFILE_FUNCTION(Cyan); // Profile the Run function

    if (!m_Window) {
        m_Logger->error("Cannot run without a valid window. Did Init() fail?");
        return;
    }

    m_LastFrameTime = glfwGetTime();

    // Main loop
    while (!glfwWindowShouldClose(m_Window)) {
        UpdateAndRenderFrame();
    }

    // Cleanup
    ShutdownImGui();
    //ShutdownGpuQueries();
    GLContext::Cleanup(m_Window);
}

void Application::UpdateAndRenderFrame()
{
    PROFILE_BLOCK("Frame", Purple);

    // CPU timing
    auto cpuStart = std::chrono::high_resolution_clock::now();

    // Clear
    {
        PROFILE_BLOCK("Render Setup", Blue);
        glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Compute deltaTime
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;

    // Process input
    {
        PROFILE_BLOCK("Handle Input", Blue);
        glfwPollEvents();
        ProcessInput(deltaTime);
    }

    // Update + Render current test
    {
        PROFILE_BLOCK("UpdateAndRenderTest", Blue);
        {
            PROFILE_BLOCK("UpdateCurrentTest", Green);
            m_TestManager.UpdateCurrentTest(static_cast<float>(deltaTime));
        }
        {
            PROFILE_BLOCK("RenderCurrentTest", Green);
            m_TestManager.RenderCurrentTest();
        }
        {
            PROFILE_BLOCK("SynchronizeCameraController", Green);
            SynchronizeCameraController();
        }
    }

    // ImGui
    {
        PROFILE_BLOCK("ImGui Rendering", Blue);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_TestManager.RenderCurrentTestImGui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    {
        PROFILE_BLOCK("SwapBuffers", Blue);
        glfwSwapBuffers(m_Window);
    }
    {
        PROFILE_BLOCK("UpdateInputManager", Blue);
        m_InputManager.Update();
    }

    // Stop CPU timing
    auto cpuEnd = std::chrono::high_resolution_clock::now();
    double cpuFrameTimeMs
        = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();

    // Show perf info
    ShowFpsAndFrameTimes(cpuFrameTimeMs);
}

void Application::ProcessInput(double deltaTime)
{
    PROFILE_FUNCTION(Yellow);

    m_CameraController.Update(static_cast<float>(deltaTime));

    // Example: Reload all shaders if 'R' is just pressed
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
        m_Logger->debug("No camera available in current test.");
    }
}

void Application::ShowFpsAndFrameTimes(double cpuFrameTimeMs)
{
    static double prevSeconds = 0.0;
    static int    frameCount = 0;

    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - prevSeconds;
    frameCount++;

    if (elapsedSeconds > 0.25) {
        prevSeconds = currentSeconds;
        double fps = static_cast<double>(frameCount) / elapsedSeconds;
        frameCount = 0;

        std::ostringstream oss;
        oss.precision(3);
        oss << std::fixed
            << "FPS: " << fps << "   "
            << "CPU: " << cpuFrameTimeMs << " ms";

        glfwSetWindowTitle(m_Window, oss.str().c_str());
    }
}

void Application::RegisterCallbacks()
{
    PROFILE_FUNCTION(Green);

    // Key
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        PROFILE_BLOCK("Key Callback", Yellow);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        app->m_InputManager.SetKey(key, isPressed);
        });

    // Cursor
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        PROFILE_BLOCK("CursorPos Callback", Yellow);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->m_CameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            app->m_CameraController.Reset();
        }
        });

    // Scroll
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        PROFILE_BLOCK("Scroll Callback", Yellow);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;

        app->m_CameraController.ProcessMouseScroll(static_cast<float>(yoffset));
        });

    // Window resize
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        PROFILE_BLOCK("WindowSize Callback", Yellow);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;

        if (height == 0) height = 1;
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
    ImGui_ImplOpenGL3_Init("#version 460");
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

//void Application::InitializeGpuQueries()
//{
//    PROFILE_BLOCK("Create GPU Queries", Yellow);
//    glGenQueries(1, &m_GpuQueryStart);
//    glGenQueries(1, &m_GpuQueryEnd);
//}
//
//void Application::ShutdownGpuQueries()
//{
//    PROFILE_BLOCK("Delete GPU Queries", Magenta);
//
//    if (m_GpuQueryStart) {
//        glDeleteQueries(1, &m_GpuQueryStart);
//        m_GpuQueryStart = 0;
//    }
//    if (m_GpuQueryEnd) {
//        glDeleteQueries(1, &m_GpuQueryEnd);
//        m_GpuQueryEnd = 0;
//    }
//}