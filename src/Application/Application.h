#pragma once

#include "Application/OpenGLContext.h"
#include "Utilities/Logger.h"
#include "Scene/Screen.h"
#include "Scene/CameraController.h"
#include "Application/InputManager.h"
#include "TestManager.h"
#include "TestMenu.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"

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

private:
    void ProcessInput(double deltaTime);
    void UpdateCameraController();
    void SetupCallbacks();

    GLFWwindow* window = nullptr;
    CameraController cameraController;
    InputManager inputManager;
    TestManager testManager;
    std::shared_ptr<TestMenu> testMenu; 
    double lastTime = 0.0;

    std::shared_ptr<spdlog::logger> logger;
};