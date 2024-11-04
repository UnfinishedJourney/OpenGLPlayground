#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "Application/OpenGLContext.h"
#include "Scene/FrameData.h"
#include "Scene/Camera.h"
#include "AllTests.h"
#include "Application/InputManager.h"
#include "Utilities/Logger.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define USING_EASY_PROFILER
#include "easy/profiler.h"
int Screen::s_Width = 960;
int Screen::s_Height = 540;
glm::mat4 FrameData::s_Projection = glm::mat4(1.0f);
glm::mat4 FrameData::s_View = glm::mat4(1.0f);
glm::vec3 FrameData::s_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos);
void glfw_onWindowSize(GLFWwindow* window, int width, int height);
void showFPS(GLFWwindow* window);

struct Application {
    GLFWwindow* window;
    Camera camera;
    CameraController cameraController;
    InputManager inputManager;
    std::shared_ptr<test::Test> currentTest;
    std::shared_ptr<test::TestMenu> testMenu;
    double lastTime;

    Application()
        : window(nullptr), camera(), cameraController(camera), lastTime(0.0) {}

    void Init();
    void Run();
    void Cleanup();
};


void Application::Init() {
    EASY_FUNCTION(profiler::colors::Magenta);
    EASY_BLOCK("App Init");
    Logger::Init();
    auto logger = Logger::GetLogger();
    logger->info("Application started.");

    window = GLContext::InitOpenGL(Screen::s_Width, Screen::s_Height, "My Application", glfw_onKey, glfw_onMouseScroll, glfw_onMouseMove, glfw_onWindowSize);

    glfwSetWindowUserPointer(window, this);
    //test::Test::InitializeRenderer();
    glfwSetWindowSizeCallback(window, glfw_onWindowSize);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    auto renderer = std::make_shared<Renderer>();
    currentTest = nullptr;
    testMenu = std::make_shared<test::TestMenu>(currentTest, renderer);
    currentTest = testMenu;
    testMenu->RegisterTest<test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<test::TestSimpleCube>("Simple Cube");
    testMenu->RegisterTest<test::TestSkyBox>("SkyBox");
    testMenu->RegisterTest<test::TestLights>("Lights");
    //testMenu->RegisterTest<test::TestAssimp>("Assimp");
    //testMenu->RegisterTest<test::TestInstance>("Instances");
    //testMenu->RegisterTest<test::TestPBR>("PBR");
    //testMenu->RegisterTest<test::TestComputeShader>("Compute");
    //testMenu->RegisterTest<test::TestDamagedHelmet>("Helmet");

    FrameData::s_View = camera.GetViewMatrix();
    FrameData::s_Projection = glm::perspective(glm::radians(camera.GetFOV()), (float)Screen::s_Width / (float)Screen::s_Height, 0.1f, 100.0f);
    EASY_END_BLOCK;
}

void Application::Run() {
    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        showFPS(window);
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        if (inputManager.IsKeyPressed(GLFW_KEY_W)) {
            cameraController.ProcessKeyboard(CameraMovement::UP, deltaTime);
        }
        if (inputManager.IsKeyPressed(GLFW_KEY_S)) {
            cameraController.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
        }
        if (inputManager.IsKeyPressed(GLFW_KEY_A)) {
            cameraController.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
        }
        if (inputManager.IsKeyPressed(GLFW_KEY_D)) {
            cameraController.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
        }

        FrameData::s_View = camera.GetViewMatrix();
        currentTest->Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (currentTest) {
            currentTest->OnUpdate(deltaTime);
            currentTest->OnRender();
            ImGui::Begin("Test");
            if (currentTest != testMenu && ImGui::Button("-<")) {
                currentTest = testMenu;
            }
            currentTest->OnImGuiRender();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void Application::Cleanup() {
    GLContext::Cleanup(window);
}

int main() {
    EASY_PROFILER_ENABLE;
    profiler::startListen();
    Application app;
    app.Init();
    app.Run();
    app.Cleanup();
    profiler::dumpBlocksToFile("profile_data.prof");
    return 0;
}

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    app->inputManager.SetKey(key, isPressed);
}

void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos) {
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

void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->cameraController.ProcessMouseScroll(static_cast<float>(deltaY));
}

void glfw_onWindowSize(GLFWwindow* window, int width, int height) {

    if (height == 0)
        height = 1; 

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    Screen::s_Width = width;
    Screen::s_Height = height;

    glViewport(0, 0, width, height);

    FrameData::s_Projection = glm::perspective(glm::radians(app->camera.GetFOV()),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f, 100.0f);
}

void showFPS(GLFWwindow* window) {
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double elapsedSeconds;
    double currentSeconds = glfwGetTime();

    elapsedSeconds = currentSeconds - previousSeconds;
    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = (double)frameCount / elapsedSeconds;
        double msPerFrame = 1000.0 / fps;

        std::ostringstream outs;
        outs.precision(3);
        outs << std::fixed
            << "FPS: " << fps << "    "
            << "Frame Time: " << msPerFrame << " (ms)";
        glfwSetWindowTitle(window, outs.str().c_str());
        frameCount = 0;
    }

    frameCount++;
}