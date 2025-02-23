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
 * Core application class handling initialization, run loop, and cleanup.
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
    GLFWwindow* window_ = nullptr;
    Scene::CameraController cameraController_;
    InputManager inputManager_;
    TestManager testManager_;
    std::shared_ptr<TestMenu> testMenu_ = nullptr;
    double lastFrameTime_ = 0.0;
    std::shared_ptr<spdlog::logger> logger_;
};