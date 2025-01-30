#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <imgui_impl_glfw.h>
#include "Scene/CameraController.h"
#include "Application/InputManager.h"
#include "TestManager.h"
#include "TestMenu.h"

/**
 * @class Application
 * @brief Handles core initialization, the main run loop, and cleanup of the OpenGL application.
 */
class Application
{
public:
    /**
     * @brief Default constructor that sets up the logger and configures the camera controller.
     */
    Application();

    /**
     * @brief Destructor that ensures proper cleanup.
     */
    ~Application();

    /**
     * @brief Initializes the application (GLFW window, OpenGL context, ImGui, tests, etc.).
     * @return True if initialization was successful, false otherwise.
     */
    bool Init();

    /**
     * @brief Enters the main run loop of the application.
     */
    void Run();

private:
    // --------------------------------------------------
    // Helper methods
    // --------------------------------------------------

    /**
     * @brief Registers various GLFW callbacks for input handling and window events.
     */
    void RegisterCallbacks();

    /**
     * @brief Initializes the Dear ImGui library.
     */
    void InitializeImGui();

    /**
     * @brief Shuts down the Dear ImGui library.
     */
    void ShutdownImGui();

    /**
     * @brief Processes per-frame logic: input handling, test updates, rendering, etc.
     */
    void UpdateAndRenderFrame();

    /**
     * @brief Processes keyboard/mouse input each frame.
     * @param deltaTime The elapsed time since the previous frame, in seconds.
     */
    void ProcessInput(double deltaTime);

    /**
     * @brief Synchronizes the current camera from the active test with the CameraController.
     */
    void SynchronizeCameraController();

    /**
     * @brief Updates the window title with FPS and CPU frame time information.
     * @param cpuFrameTimeMs The time the CPU spent on the last frame, in milliseconds.
     */
    void ShowFpsAndFrameTimes(double cpuFrameTimeMs);

private:
    // --------------------------------------------------
    // Member variables
    // --------------------------------------------------
    GLFWwindow*                 m_Window = nullptr;   ///< Pointer to the GLFW window.
    CameraController            m_CameraController;              ///< Controls the active camera.
    InputManager                m_InputManager;                  ///< Manages keyboard & mouse input.
    TestManager                 m_TestManager;                   ///< Manages the currently active test scene.
    std::shared_ptr<TestMenu>   m_TestMenu = nullptr;   ///< Menu that allows switching between tests.
    double                      m_LastFrameTime = 0.0;       ///< Time of the last frame, used for deltaTime.

    std::shared_ptr<spdlog::logger> m_Logger;                    ///< The local logger (obtained from global Logger).
};