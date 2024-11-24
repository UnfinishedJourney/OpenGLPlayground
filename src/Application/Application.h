#pragma once

#include "Application/OpenGLContext.h"
#include "Utilities/Logger.h"
#include "AllTests.h"
#include "Scene/Screen.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Scene/CameraController.h"
#include "Application/InputManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <memory>

class Application {
public:
    Application();
    ~Application();

    void Init();
    void Run();
    void Cleanup();

private:
    void ProcessInput(double deltaTime);
    void UpdateCameraController();
    void SetupCallbacks();

    GLFWwindow* window = nullptr;
    CameraController cameraController;
    InputManager inputManager;
    std::shared_ptr<test::Test> currentTest = nullptr;
    std::shared_ptr<test::TestMenu> testMenu = nullptr;
    double lastTime = 0.0;

    std::shared_ptr<spdlog::logger> logger;
};