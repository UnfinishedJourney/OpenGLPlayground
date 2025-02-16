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
 * Handles core initialization, the main run loop, and cleanup of the OpenGL application.
 */
class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();

private:
    void RegisterCallbacks();
    void InitializeImGui();
    void ShutdownImGui();
    void UpdateAndRenderFrame();
    void ProcessInput(double deltaTime);
    void SynchronizeCameraController();
    void ShowFpsAndFrameTimes(double cpuFrameTimeMs);

private:
    GLFWwindow* window_ = nullptr;                   // GLFW window pointer
    Scene::CameraController cameraController_;       // Active camera controller
    InputManager inputManager_;                      // Manages input
    TestManager testManager_;                        // Manages test scenes
    std::shared_ptr<TestMenu> testMenu_ = nullptr;     // Test menu for switching tests
    double lastFrameTime_ = 0.0;                       // Last frame time for deltaTime calculation

    std::shared_ptr<spdlog::logger> logger_;         // Local logger instance
};