#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

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

#include <easy/profiler.h>
#include <sstream>
#include <memory>

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos);
void glfw_onWindowSize(GLFWwindow* window, int width, int height);
void showFPS(GLFWwindow* window);

class Application {
public:
    Application();
    ~Application();

    void Init();
    void Run();
    void Cleanup();

    GLFWwindow* window;
    CameraController cameraController;
    InputManager inputManager;
    std::shared_ptr<test::Test> currentTest;
    std::shared_ptr<test::TestMenu> testMenu;
    double lastTime;

private:
    void ProcessInput(double deltaTime);
    void UpdateCameraController();
    std::shared_ptr<spdlog::logger> logger;
};

Application::Application()
    : window(nullptr), inputManager(), cameraController(inputManager), lastTime(0.0)
{
    cameraController.SetCamera(nullptr);
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

    window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application",
        glfw_onKey, glfw_onMouseScroll, glfw_onMouseMove, glfw_onWindowSize);

    if (!window) {
        logger->error("Failed to initialize OpenGL context.");
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Get initial window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    currentTest = nullptr;
    testMenu = std::make_shared<test::TestMenu>(currentTest);
    currentTest = testMenu;

    //testMenu->RegisterTest<test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<test::TestSimpleCube>("Simple Cube");
    // testMenu->RegisterTest<test::TestSkyBox>("SkyBox");
    //testMenu->RegisterTest<test::TestLights>("Lights");
    // testMenu->RegisterTest<test::TestSkyBoxReflection>("SkyboxReflection");
    // Add more tests as needed
}

void Application::Run()
{
    if (!window) {
        logger->error("Cannot run application without a valid window.");
        return;
    }

    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.3, 0.2, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        showFPS(window);

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        ProcessInput(deltaTime);

        // Update current test
        if (currentTest) {
            currentTest->OnUpdate(static_cast<float>(deltaTime));
            
            // Render
            currentTest->OnRender();

            // Render ImGui
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

            // Swap buffers
            glfwSwapBuffers(window);
            inputManager.Update(); // Update key states at the end of the frame
        }
    }
}

void Application::Cleanup()
{
    GLContext::Cleanup(window);
    logger->info("Application terminated gracefully.");
}

void Application::ProcessInput(double deltaTime)
{
    cameraController.Update(deltaTime);
    if (inputManager.WasKeyJustPressed(GLFW_KEY_R)) {
        logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

//need to get camera differently
void Application::UpdateCameraController()
{
    if (currentTest) {
        std::shared_ptr<Camera> camera = currentTest->GetCamera();
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

int main()
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();

    Application app;
    app.Init();
    app.Run();

    profiler::dumpBlocksToFile("profile_data.prof");
    return 0;
}

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    app->inputManager.SetKey(key, isPressed);
}

void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        app->cameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        app->cameraController.Reset();
    }
}

void glfw_onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->cameraController.ProcessMouseScroll(static_cast<float>(yoffset));
}

void glfw_onWindowSize(GLFWwindow* window, int width, int height)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (height == 0)
        height = 1;

    // Update the viewport
    glViewport(0, 0, width, height);

    // Notify the current test
    if (app->currentTest)
    {
        app->currentTest->OnWindowResize(width, height);
    }

    // Update screen resolution and physical parameters
    Screen::SetResolution(width, height);

    app->cameraController.UpdateFOV();
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