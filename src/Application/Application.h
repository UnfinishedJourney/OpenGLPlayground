#pragma once

#include <memory>
#include <string>

#include "Scene/CameraController.h"
#include "Application/InputManager.h"
#include "TestManager.h"
#include "TestMenu.h"
#include "Utilities/Logger.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

/**
 * @brief Main Application class. Handles core initialization, the run loop, and cleanup.
 */
class Application
{
public:
    Application();
    ~Application();

    void Init();
    void Run();

private:
    // Helpers
    void RegisterCallbacks();
    void InitializeImGui();
    void ShutdownImGui();
    //void InitializeGpuQueries();
    //void ShutdownGpuQueries();
    void UpdateAndRenderFrame();
    void ProcessInput(double deltaTime);
    void SynchronizeCameraController();
    void ShowFpsAndFrameTimes(double cpuFrameTimeMs);

private:
    GLFWwindow* m_Window = nullptr;
    CameraController            m_CameraController;
    InputManager                m_InputManager;
    TestManager                 m_TestManager;
    std::shared_ptr<TestMenu>   m_TestMenu = nullptr;
    double                      m_LastFrameTime = 0.0;

    // GPU Queries to measure GPU frame time
    //GLuint                      m_GpuQueryStart = 0;
    //GLuint                      m_GpuQueryEnd = 0;

    // Logging
    std::shared_ptr<spdlog::logger> m_Logger;
};