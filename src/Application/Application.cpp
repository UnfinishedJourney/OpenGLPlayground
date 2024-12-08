#include "Application.h"
#include "Utilities/Logger.h"
#include <sstream>
#define USING_EASY_PROFILER
#include <easy/profiler.h>
#include "AllTests.h"

Application::Application()
    : cameraController(inputManager) {
    Logger::Init();
    logger = Logger::GetLogger();
    logger->info("Application started.");
}

Application::~Application() {
    logger->info("Application terminated gracefully.");
}

void Application::Init() {
    EASY_FUNCTION(profiler::colors::Magenta);
    EASY_BLOCK("App Init");
    window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application");

    if (!window) {
        logger->error("Failed to initialize OpenGL context.");
        return;
    }

    glfwSetWindowUserPointer(window, this);
    SetupCallbacks();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize ImGui
    logger->info("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 460";
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    logger->info("ImGui initialized successfully.");

    // Initialize TestMenu and register tests
    testMenu = std::make_unique<TestMenu>(testManager);
    //testMenu->RegisterTest("Simple Cube", []() { return std::make_shared<TestSimpleCube>(); });
    testMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
    testMenu->RegisterTest("Compute", []() { return std::make_shared<TestComputeShader>(); });
    testMenu->RegisterTest("Helmet", []() { return std::make_shared<TestDamagedHelmet>(); });
    testMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
    //testMenu->RegisterTest("Terrain", []() { return std::make_shared<TestTerrain>(); });
    // Add more tests as needed

    // Register the TestMenuTest
    testManager.RegisterTest("Test Menu", [this]() {
        return std::make_shared<TestMenuTest>(*testMenu);
        });

    // Start with the TestMenu
    testManager.SwitchTest("Test Menu");
    EASY_END_BLOCK;
}

void showFPS(GLFWwindow* window) {
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - previousSeconds;

    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = static_cast<double>(frameCount) / elapsedSeconds;
        double msPerFrame = 1000.0 / fps;

        std::ostringstream outs;
        outs.precision(3);
        outs << std::fixed
            << "FPS: " << fps << "    "
            << "Frame Time: " << msPerFrame << " ms";

        glfwSetWindowTitle(window, outs.str().c_str());
        frameCount = 0;
    }

    frameCount++;
}

void Application::Run() {
    if (!window) {
        logger->error("Cannot run application without a valid window.");
        return;
    }

    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        showFPS(window);

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        ProcessInput(deltaTime);

        testManager.UpdateCurrentTest(static_cast<float>(deltaTime));
        testManager.RenderCurrentTest();
        UpdateCameraController();
        // ImGui Rendering
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        testManager.RenderCurrentTestImGui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        inputManager.Update(); // Update key states at the end of the frame
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    GLContext::Cleanup(window);
}

void Application::ProcessInput(double deltaTime) {
    cameraController.Update(deltaTime);
    if (inputManager.WasKeyJustPressed(GLFW_KEY_R)) {
        logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

void Application::UpdateCameraController() {
    auto camera = testManager.GetCurrentCamera();
    cameraController.SetCamera(camera);
    if (camera) {
        logger->debug("CameraController updated with new Camera from current Test.");
    }
    else {
        logger->debug("Current Test does not have a Camera.");
    }
}

void Application::SetupCallbacks() {
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        app->inputManager.SetKey(key, isPressed);
        });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->cameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            app->cameraController.Reset();
        }
        });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->cameraController.ProcessMouseScroll(static_cast<float>(yoffset));
        });

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (height == 0)
            height = 1;

        glViewport(0, 0, width, height);
        Screen::SetResolution(width, height);

        app->testManager.HandleWindowResize(width, height);
        app->cameraController.UpdateFOV();
        });
}

