#pragma once

//#include <GLFW/glfw3.h>
#include <memory>
#include "Scene/CameraController.h"
#include "Application/InputManager.h"
#include "TestManager.h"
#include "TestMenu.h"
#include "Utilities/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

/**
 * @brief Main Application class handling the core run loop and initialization.
 */
class Application
{
public:
    Application();
    ~Application();

    void Init();
    void Run();

private:
    // Initialization Helpers
    void RegisterCallbacks();
    void InitializeImGui();
    void ShutdownImGui();
    void InitializeGpuQueries();
    void ShutdownGpuQueries();

    // Per-frame Helpers
    void UpdateAndRenderFrame();
    void HandleInput(double deltaTime);
    void SynchronizeCameraController();
    void ShowFpsAndFrameTimes(double cpuFrameTimeMs, double gpuFrameTimeMs);

private:
    GLFWwindow* m_Window{ nullptr };
    CameraController m_CameraController;
    InputManager m_InputManager;
    TestManager m_TestManager;
    std::shared_ptr<TestMenu> m_TestMenu{ nullptr };

    // Timing
    double m_LastFrameTime{ 0.0 };

    // GPU Query Objects
    GLuint m_GpuQueryStart{ 0 };
    GLuint m_GpuQueryEnd{ 0 };

    // Logger
    std::shared_ptr<spdlog::logger> m_Logger;
};