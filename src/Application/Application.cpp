#include "Application.h"
#include <easy/profiler.h>

Application::Application()
    : cameraController(inputManager)
{
    Logger::Init();
    logger = Logger::GetLogger();
    logger->info("Application started.");
}

Application::~Application()
{
    Cleanup();
}

void Application::Init()
{
    EASY_FUNCTION();

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

    currentTest = nullptr;
    testMenu = std::make_shared<test::TestMenu>(currentTest);
    currentTest = testMenu;

    // Register tests
    testMenu->RegisterTest<test::TestSimpleCube>("Simple Cube");
    testMenu->RegisterTest<test::TestLights>("Lights");
    testMenu->RegisterTest<test::TestTerrain>("Terrain");
    // Add more tests as needed
}

void showFPS(GLFWwindow* window)
{
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

void Application::Run()
{
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

        if (currentTest) {
            currentTest->OnUpdate(static_cast<float>(deltaTime));
            currentTest->OnRender();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Test");
            if (currentTest != testMenu && ImGui::Button("<- Back")) {
                currentTest->OnExit();
                currentTest = testMenu;
                currentTest->OnEnter();
            }
            currentTest->OnImGuiRender();
            UpdateCameraController();
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            inputManager.Update(); // Update key states at the end of the frame
        }
    }
}

void Application::Cleanup()
{
    logger->info("Starting application cleanup.");

    logger->info("Shutting down ImGui...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    logger->debug("ImGui shutdown successfully.");

    GLContext::Cleanup(window);
    logger->info("We are finished.");
}

void Application::ProcessInput(double deltaTime)
{
    cameraController.Update(deltaTime);
    if (inputManager.WasKeyJustPressed(GLFW_KEY_R)) {
        logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

void Application::UpdateCameraController()
{
    if (currentTest) {
        auto camera = currentTest->GetCamera();
        cameraController.SetCamera(camera);
        if (camera) {
            logger->debug("CameraController updated with new Camera from Test.");
        }
        else {
            logger->debug("Current Test does not have a Camera.");
        }
    }
    else {
        cameraController.SetCamera(nullptr);
        logger->debug("No active Test to retrieve Camera from.");
    }
}

void Application::SetupCallbacks()
{
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

        if (app->currentTest)
            app->currentTest->OnWindowResize(width, height);

        Screen::SetResolution(width, height);
        app->cameraController.UpdateFOV();
        });
}