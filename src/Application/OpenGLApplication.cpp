#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Application/OpenGLContext.h"
#include "Utilities/Logger.h"
#include "AllTests.h"
#include "Scene/FrameData.h"
#include "Scene/Camera.h"
#include "Renderer/Renderer.h"
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
    Camera camera;
    CameraController cameraController;
    InputManager inputManager;
    std::shared_ptr<test::Test> currentTest;
    std::shared_ptr<test::TestMenu> testMenu;
    double lastTime;

private:
    void ProcessInput(double deltaTime);
    std::shared_ptr<spdlog::logger> logger;
};


Application::Application()
    : window(nullptr), camera(), inputManager(), cameraController(camera, inputManager), lastTime(0.0)
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

    window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "OpenGL Application",
        glfw_onKey, glfw_onMouseScroll, glfw_onMouseMove, glfw_onWindowSize);

    if (!window) {
        logger->error("Failed to initialize OpenGL context.");
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Initialize the Renderer singleton
    Renderer::GetInstance().Initialize();

    currentTest = nullptr;
    testMenu = std::make_shared<test::TestMenu>(currentTest);
    currentTest = testMenu;

    testMenu->RegisterTest<test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<test::TestSimpleCube>("Simple Cube");
    testMenu->RegisterTest<test::TestSkyBox>("SkyBox");
    testMenu->RegisterTest<test::TestLights>("Lights");
    // Add more tests as needed

    FrameData::s_View = camera.GetViewMatrix();
    FrameData::s_Projection = glm::perspective(glm::radians(camera.GetFOV()),
        static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height), 0.1f, 100.0f);
}

void Application::Run()
{
    if (!window) {
        logger->error("Cannot run application without a valid window.");
        return;
    }

    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        EASY_BLOCK("Main Loop");
        showFPS(window);

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        ProcessInput(deltaTime);

        FrameData::s_View = camera.GetViewMatrix();
        FrameData::s_CameraPos = camera.GetPosition();
        Renderer::GetInstance().Clear();
        //currentTest->Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (currentTest) {
            currentTest->OnUpdate(static_cast<float>(deltaTime));
            currentTest->OnRender();

            ImGui::Begin("Test");
            if (currentTest != testMenu && ImGui::Button("<- Back")) {
                currentTest->OnExit();
                currentTest = testMenu;
                currentTest->OnEnter();
            }
            currentTest->OnImGuiRender();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        EASY_END_BLOCK;
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

// Callback functions
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
    if (height == 0)
        height = 1;

    Screen::s_Width = width;
    Screen::s_Height = height;

    glViewport(0, 0, width, height);

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    FrameData::s_Projection = glm::perspective(glm::radians(app->camera.GetFOV()),
        static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
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