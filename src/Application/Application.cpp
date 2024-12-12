#include "Application.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
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

static GLuint g_queryStart = 0;
static GLuint g_queryEnd = 0;

static void showFPSAndTimes(GLFWwindow* window, double cpuFrameTimeMs, double gpuFrameTimeMs)
{
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - previousSeconds;

    frameCount++;
    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = (double)frameCount / elapsedSeconds;
        frameCount = 0;

        std::ostringstream outs;
        outs.precision(3);
        outs << std::fixed
            << "FPS: " << fps << "    "
            << "CPU: " << cpuFrameTimeMs << " ms    "
            << "GPU: " << gpuFrameTimeMs << " ms";

        glfwSetWindowTitle(window, outs.str().c_str());
    }
}

void Application::Init()
{
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

    testMenu = std::make_unique<TestMenu>(testManager);
    testMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
    testMenu->RegisterTest("Compute", []() { return std::make_shared<TestComputeShader>(); });
    testMenu->RegisterTest("Helmet", []() { return std::make_shared<TestDamagedHelmet>(); });
    testMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
    testMenu->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });

    testManager.RegisterTest("Test Menu", [this]() {
        return std::make_shared<TestMenuTest>(*testMenu);
        });
    testManager.SwitchTest("Test Menu");
    EASY_END_BLOCK;

    // Create GPU queries
    GLCall(glGenQueries(1, &g_queryStart));
    GLCall(glGenQueries(1, &g_queryEnd));
}

void Application::Run()
{
    if (!window) {
        logger->error("Cannot run application without a valid window.");
        return;
    }

    lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        EASY_BLOCK("Frame");

        // Start CPU timer
        auto cpuStart = std::chrono::high_resolution_clock::now();

        // Start GPU timer query
        GLCall(glBeginQuery(GL_TIME_ELAPSED, g_queryStart));

        GLCall(glClearColor(0.3f, 0.2f, 0.8f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        ProcessInput(deltaTime);

        testManager.UpdateCurrentTest((float)deltaTime);
        testManager.RenderCurrentTest();
        UpdateCameraController();

        // IMGUI rendering
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        testManager.RenderCurrentTestImGui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // End GPU query and start the end query
        GLCall(glEndQuery(GL_TIME_ELAPSED));

        // Another query can be started if needed, but typically one per frame is enough.
        // For simplicity, we just measure the whole frame. If needed, measure sub-parts.

        GLCall(glFinish()); // Ensure all commands completed before we read the query

        // Fetch GPU timing
        GLuint64 gpuTime = 0;
        GLCall(glGetQueryObjectui64v(g_queryStart, GL_QUERY_RESULT, &gpuTime));
        double gpuFrameTimeMs = (double)gpuTime / 1000000.0; // ns to ms

        // Swap buffers
        glfwSwapBuffers(window);
        inputManager.Update();

        // Stop CPU timer
        auto cpuEnd = std::chrono::high_resolution_clock::now();
        double cpuFrameTimeMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();

        // Show FPS and times in title
        showFPSAndTimes(window, cpuFrameTimeMs, gpuFrameTimeMs);

        EASY_END_BLOCK;
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

