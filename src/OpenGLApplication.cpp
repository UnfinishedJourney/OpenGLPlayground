#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "AllTests.h"

int Screen::s_Width = 960;
int Screen::s_Height = 540;
glm::mat4 FrameData::s_Projection = glm::mat4(1.0f);
glm::mat4 FrameData::s_View = glm::mat4(1.0f);

Camera camera;
CameraController cameraController(camera);
bool keys[4];

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos);
void showFPS(GLFWwindow* window);
void update(double elapsedTime, GLFWwindow* gWindow, CameraController& cameraController);

const double ZOOM_SENSITIVITY = -1.0;
const float MOVE_SPEED = 0.001f; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

GLFWwindow* GLInit()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window;

    window = glfwCreateWindow(Screen::s_Width, Screen::s_Height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_onKey);
    glfwSetScrollCallback(window, glfw_onMouseScroll);
    glfwSetCursorPosCallback(window, glfw_onMouseMove);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << glGetString(GL_VERSION) << std::endl;
    GLCall(glEnable(GL_DEPTH_TEST));

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    return window;
}

void GLEnd(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(void)
{
    GLFWwindow* window = GLInit();


    Renderer renderer;
    test::Test* currentTest = nullptr;
    test::TestMenu* testMenu = new test::TestMenu(currentTest);
    currentTest = testMenu;
    testMenu->RegisterTest<test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<test::TestSimpleCube>("Simple Cube");
    testMenu->RegisterTest<test::TestAssimp>("Assimp");
    testMenu->RegisterTest<test::TestLights>("Lights");

    double lastTime = glfwGetTime();
    FrameData::s_View = camera.GetViewMatrix();
    FrameData::s_Projection = glm::perspective(glm::radians(camera.GetFOV()), (float)Screen::s_Width / (float)Screen::s_Height, 0.1f, 100.0f);

    /* Main render loop */
    while (!glfwWindowShouldClose(window))
    {
        showFPS(window);
        glfwPollEvents();
        renderer.Clear();

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;

        // Poll for and process events
        glfwPollEvents();
        update(deltaTime, window, cameraController);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        FrameData::s_View = camera.GetViewMatrix();

        if (currentTest)
        {
            currentTest->OnUpdate(deltaTime);
            currentTest->OnRender();
            ImGui::Begin("Test");
            if (currentTest != testMenu && ImGui::Button("-<"))
            {
                delete currentTest;
                currentTest = testMenu;
            }
            currentTest->OnImGuiRender();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        lastTime = currentTime;
    }

    if (currentTest != testMenu)
        delete testMenu;
    delete currentTest;

    GLEnd(window);
    exit(EXIT_SUCCESS);
    return 0;
}

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS)
            keys[CameraMovement::UP] = true;
        else if (action == GLFW_RELEASE)
            keys[CameraMovement::UP] = false;
    }

    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS)
            keys[CameraMovement::DOWN] = true;
        else if (action == GLFW_RELEASE)
            keys[CameraMovement::DOWN] = false;
    }

    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS)
            keys[CameraMovement::LEFT] = true;
        else if (action == GLFW_RELEASE)
            keys[CameraMovement::LEFT] = false;
    }

    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS)
            keys[CameraMovement::RIGHT] = true;
        else if (action == GLFW_RELEASE)
            keys[CameraMovement::RIGHT] = false;
    }
}

void glfw_onMouseMove(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        cameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }
}

void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY) {
    cameraController.ProcessMouseScroll(deltaY);
}

void update(double elapsedTime, GLFWwindow* gWindow, CameraController& cameraController) {

    if (keys[CameraMovement::UP]) {
        cameraController.ProcessKeyboard(UP, elapsedTime);
    }
    if (keys[CameraMovement::DOWN]) {
        cameraController.ProcessKeyboard(DOWN, elapsedTime);
    }
    if (keys[CameraMovement::LEFT]) {
        cameraController.ProcessKeyboard(LEFT, elapsedTime);
    }
    if (keys[CameraMovement::RIGHT]) {
        cameraController.ProcessKeyboard(RIGHT, elapsedTime);
    }

    FrameData::s_View = camera.GetViewMatrix();
    FrameData::s_Projection = glm::perspective(
        glm::radians(camera.GetFOV()),
        static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height),  
        0.1f,  
        100.0f 
    );
}


void showFPS(GLFWwindow* window)
{
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double elapsedSeconds;
    double currentSeconds = glfwGetTime();

    elapsedSeconds = currentSeconds - previousSeconds;
    //limit text update 4 times per second
    if (elapsedSeconds > 0.25)
    {
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