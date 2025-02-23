#include "Application.h"
#include "Application/OpenGLContext.h"
#include "Renderer/Renderer.h"
#include "Scene/Screen.h"
#include "AllTests.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"

#include <chrono>
#include <sstream>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

Application::Application()
    : cameraController_(inputManager_) {
    Logger::Init();
    logger_ = Logger::GetLogger();
    logger_->info("Application constructed.");
}

Application::~Application() {
    logger_->info("Application destructor called.");
}

bool Application::Init() {
    PROFILE_FUNCTION(Magenta);
    window_ = GLContext::InitOpenGL(Screen::width_, Screen::height_, "OpenGL Application");
    if (!window_) {
        logger_->critical("Failed to initialize the OpenGL context.");
        return false;
    }

    glfwSetWindowUserPointer(window_, this);
    RegisterCallbacks();
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    InitializeImGui();

    {
        PROFILE_BLOCK("Initialize TestMenu", Yellow);
        testMenu_ = std::make_shared<TestMenu>(testManager_);
        testMenu_->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
        testMenu_->RegisterTest("ClearColor", []() { return std::make_shared<TestClearColor>(); });
        testMenu_->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
        testMenu_->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });
        testMenu_->RegisterTest("PBRHelmet", []() { return std::make_shared<TestDamagedHelmet>(); });
        testMenu_->RegisterTest("TestShadows", []() { return std::make_shared<TestShadows>(); });
        testManager_.RegisterTest("Test Menu", [this]() {
            return std::make_shared<TestMenuTest>(testMenu_);
            });
        testManager_.SwitchTest("Test Menu");

        auto camera = testManager_.GetCurrentCamera();
        cameraController_.SetCamera(camera);
    }

    logger_->info("Application initialized successfully.");
    return true;
}

void Application::Run() {
    PROFILE_FUNCTION(Cyan);
    if (!window_) {
        logger_->error("Invalid window. Did you call Init() successfully?");
        return;
    }

    lastFrameTime_ = glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
        UpdateAndRenderFrame();
    }
    ShutdownImGui();
    GLContext::Cleanup(window_);
}

void Application::UpdateAndRenderFrame() {
    PROFILE_BLOCK("Frame", Purple);
    auto cpuStartTime = std::chrono::steady_clock::now();

    PROFILE_BLOCK("Render Setup", Blue);
    glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastFrameTime_;
    lastFrameTime_ = currentTime;

    PROFILE_BLOCK("Handle Input", Blue);
    glfwPollEvents();
    ProcessInput(deltaTime);

    PROFILE_BLOCK("UpdateAndRenderTest", Blue);
    PROFILE_BLOCK("UpdateCurrentTest", Green);
    testManager_.UpdateCurrentTest(static_cast<float>(deltaTime));
    PROFILE_BLOCK("RenderCurrentTest", Green);
    testManager_.RenderCurrentTest();
    PROFILE_BLOCK("SynchronizeCameraController", Green);
    SynchronizeCameraController();

    PROFILE_BLOCK("ImGui Rendering", Blue);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    testManager_.RenderCurrentTestImGui();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    PROFILE_BLOCK("SwapBuffers", Blue);
    glfwSwapBuffers(window_);

    PROFILE_BLOCK("UpdateInputManager", Blue);
    inputManager_.Update();

    auto cpuEndTime = std::chrono::steady_clock::now();
    double cpuFrameTimeMs = std::chrono::duration<double, std::milli>(cpuEndTime - cpuStartTime).count();
    ShowFpsAndFrameTimes(cpuFrameTimeMs);
}

void Application::ProcessInput(double deltaTime) {
    PROFILE_FUNCTION(Yellow);
    cameraController_.Update(static_cast<float>(deltaTime));
    if (inputManager_.WasKeyJustPressed(GLFW_KEY_R)) {
        logger_->info("Reloading all shaders...");
        graphics::ShaderManager::GetInstance().ReloadAllShaders();
    }
}

void Application::SynchronizeCameraController() {
    PROFILE_FUNCTION(Yellow);
    auto camera = testManager_.GetCurrentCamera();
    cameraController_.SetCamera(camera);
    if (camera) {
        logger_->debug("CameraController synchronized with current test camera.");
    }
    else {
        logger_->debug("No camera available in current test.");
    }
}

void Application::ShowFpsAndFrameTimes(double cpuFrameTimeMs) {
    static double prevSeconds = 0.0;
    static int frameCount = 0;
    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - prevSeconds;
    ++frameCount;

    if (elapsedSeconds > 0.25) {
        prevSeconds = currentSeconds;
        double fps = static_cast<double>(frameCount) / elapsedSeconds;
        frameCount = 0;
        std::ostringstream oss;
        oss.precision(3);
        oss << std::fixed << "FPS: " << fps << "   CPU: " << cpuFrameTimeMs << " ms";
        glfwSetWindowTitle(window_, oss.str().c_str());
    }
}

void Application::RegisterCallbacks() {
    PROFILE_FUNCTION(Green);
    glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        PROFILE_BLOCK("Key Callback", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        app->inputManager_.SetKey(key, isPressed);
        });

    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
        PROFILE_BLOCK("CursorPos Callback", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->cameraController_.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            app->cameraController_.Reset();
        }
        });

    glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset) {
        PROFILE_BLOCK("Scroll Callback", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;
        app->cameraController_.ProcessMouseScroll(static_cast<float>(yoffset));
        });

    glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
        PROFILE_BLOCK("WindowSize Callback", Yellow);
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);
        Screen::SetResolution(width, height);
        app->testManager_.HandleWindowResize(width, height);
        app->cameraController_.UpdateFOV();
        });
}

void Application::InitializeImGui() {
    PROFILE_BLOCK("Initialize ImGui", Yellow);
    logger_->info("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    ImGui::StyleColorsDark();
    logger_->info("ImGui initialized successfully.");
}

void Application::ShutdownImGui() {
    PROFILE_BLOCK("Shutdown ImGui", Magenta);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}