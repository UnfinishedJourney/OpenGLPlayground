#include "Application.h"

// Standard library
#include <chrono>
#include <sstream>

// Engine / App includes
#include "Application/OpenGLContext.h"
#include "Renderer/Renderer.h"
#include "Scene/Screen.h"
#include "AllTests.h"                  // Contains the definitions for TestLights, TestClearColor, etc.
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"

// ImGui
#include <imgui.h>
#include <imgui_impl_opengl3.h>

/**
 * @brief Constructs the Application, initializes the global logger, and configures the CameraController.
 */
Application::Application()
    : m_CameraController(m_InputManager)
{
    PROFILE_FUNCTION(Magenta); // Profile the constructor

    // Initialize the global logger once
    Logger::Init();
    m_Logger = Logger::GetLogger();

    m_Logger->info("Application constructed.");
}

/**
 * @brief Destructor that ensures all resources are properly freed.
 */
Application::~Application()
{
    PROFILE_FUNCTION(Magenta); // Profile the destructor
    m_Logger->info("Application destructor called.");
}

/**
 * @brief Initializes the GLFW window, OpenGL context, ImGui, and test menu.
 * @return True if everything initialized correctly, false otherwise.
 */
bool Application::Init()
{
    PROFILE_FUNCTION(Magenta); // Profile the Init function

    // 1. Initialize the GLFW & OpenGL context
    m_Window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application");
    if (!m_Window)
    {
        m_Logger->critical("Failed to initialize the OpenGL context.");
        return false;
    }

    // Store a pointer to 'this' so we can retrieve it in GLFW callbacks
    glfwSetWindowUserPointer(m_Window, this);

    // 2. Register input/resize callbacks
    RegisterCallbacks();

    // 3. Set up the mouse cursor mode (e.g., normal, hidden, or disabled)
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // 4. Initialize ImGui
    InitializeImGui();

    // 5. Create and register tests via a TestMenu
    {
        PROFILE_BLOCK("Initialize TestMenu", Yellow);

        m_TestMenu = std::make_shared<TestMenu>(m_TestManager);

        // Register your tests here
        m_TestMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
        m_TestMenu->RegisterTest("ClearColor", []() { return std::make_shared<TestClearColor>(); });
        m_TestMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
        m_TestMenu->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });
        m_TestMenu->RegisterTest("PBRHelmet", []() { return std::make_shared<TestDamagedHelmet>(); });

        // Register the menu itself as a "test"
        m_TestManager.RegisterTest("Test Menu", [this]() {
            return std::make_shared<TestMenuTest>(m_TestMenu);
            });

        // Switch to displaying the test menu by default
        m_TestManager.SwitchTest("Test Menu");
    }

    m_Logger->info("Application initialized successfully.");
    return true;
}

/**
 * @brief Runs the main application loop until the user closes the window.
 */
void Application::Run()
{
    PROFILE_FUNCTION(Cyan); // Profile the Run function

    if (!m_Window)
    {
        m_Logger->error("Invalid window. Did you call Init() successfully?");
        return;
    }

    // Initialize frame timing
    m_LastFrameTime = glfwGetTime();

    // Main loop
    while (!glfwWindowShouldClose(m_Window))
    {
        UpdateAndRenderFrame();
    }

    // Cleanup
    ShutdownImGui();
    GLContext::Cleanup(m_Window);
}

/**
 * @brief One iteration of the main loop: handle input, update tests, render, etc.
 */
void Application::UpdateAndRenderFrame()
{
    PROFILE_BLOCK("Frame", Purple);

    // Record the CPU start time for measuring CPU frame time
    auto cpuStartTime = std::chrono::steady_clock::now();

    // 1. Clear the screen (color and depth)
    {
        PROFILE_BLOCK("Render Setup", Blue);
        glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // 2. Compute deltaTime (in seconds)
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;

    // 3. Process input
    {
        PROFILE_BLOCK("Handle Input", Blue);
        glfwPollEvents();
        ProcessInput(deltaTime);
    }

    // 4. Update and render the current test
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

    // 5. ImGui-related calls
    {
        PROFILE_BLOCK("ImGui Rendering", Blue);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render any ImGui elements the current test wants to show
        m_TestManager.RenderCurrentTestImGui();

        // Render ImGui draw data
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // 6. Swap buffers to present to the screen
    {
        PROFILE_BLOCK("SwapBuffers", Blue);
        glfwSwapBuffers(m_Window);
    }

    // 7. Update the input manager state (e.g., track 'just pressed' keys)
    {
        PROFILE_BLOCK("UpdateInputManager", Blue);
        m_InputManager.Update();
    }

    // Measure CPU frame time
    auto cpuEndTime = std::chrono::steady_clock::now();
    double cpuFrameTimeMs = std::chrono::duration<double, std::milli>(cpuEndTime - cpuStartTime).count();

    // 8. Show performance info in the window title
    ShowFpsAndFrameTimes(cpuFrameTimeMs);
}

/**
 * @brief Processes input (keyboard, mouse) for each frame.
 * @param deltaTime Time since last frame in seconds.
 */
void Application::ProcessInput(double deltaTime)
{
    PROFILE_FUNCTION(Yellow);

    // Update the camera controller
    m_CameraController.Update(static_cast<float>(deltaTime));

    // Reload all shaders if 'R' is just pressed
    if (m_InputManager.WasKeyJustPressed(GLFW_KEY_R))
    {
        m_Logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

/**
 * @brief Synchronizes the CameraController with the camera from the current test.
 */
void Application::SynchronizeCameraController()
{
    PROFILE_FUNCTION(Yellow);

    auto camera = m_TestManager.GetCurrentCamera();
    m_CameraController.SetCamera(camera);

    if (camera)
    {
        m_Logger->debug("CameraController synchronized with current test camera.");
    }
    else
    {
        m_Logger->debug("No camera available in current test.");
    }
}

/**
 * @brief Updates the window title with FPS and CPU frame time data.
 * @param cpuFrameTimeMs Time spent on the last frame by the CPU, in milliseconds.
 */
void Application::ShowFpsAndFrameTimes(double cpuFrameTimeMs)
{
    static double prevSeconds = 0.0;
    static int frameCount = 0;

    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - prevSeconds;
    frameCount++;

    // Update every 0.25 seconds
    if (elapsedSeconds > 0.25)
    {
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

/**
 * @brief Registers GLFW callbacks for keyboard, mouse, scroll, and window resizing.
 */
void Application::RegisterCallbacks()
{
    PROFILE_FUNCTION(Green);

    // Key callback
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            PROFILE_BLOCK("Key Callback", Yellow);

            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (!app) return;

            // Close window on ESC
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
            app->m_InputManager.SetKey(key, isPressed);
        });

    // Mouse move callback
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
        {
            PROFILE_BLOCK("CursorPos Callback", Yellow);

            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (!app) return;

            // Example: if right mouse button is held, capture the mouse and update camera
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                app->m_CameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                app->m_CameraController.Reset();
            }
        });

    // Mouse scroll callback
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            PROFILE_BLOCK("Scroll Callback", Yellow);

            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (!app) return;

            app->m_CameraController.ProcessMouseScroll(static_cast<float>(yoffset));
        });

    // Window resize callback
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            PROFILE_BLOCK("WindowSize Callback", Yellow);

            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (!app) return;

            // Avoid division by zero
            if (height == 0) height = 1;

            // Adjust the viewport and screen resolution
            glViewport(0, 0, width, height);
            Screen::SetResolution(width, height);

            // Notify the active test and camera controller of the new size
            app->m_TestManager.HandleWindowResize(width, height);
            app->m_CameraController.UpdateFOV();
        });
}

/**
 * @brief Initializes Dear ImGui for rendering within this application.
 */
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

/**
 * @brief Shuts down Dear ImGui to clean up resources.
 */
void Application::ShutdownImGui()
{
    PROFILE_BLOCK("Shutdown ImGui", Magenta);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}