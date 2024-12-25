#include "Application.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <sstream>
#include "AllTests.h"
#include "Utilities/ProfilerMacros.h" 

Application::Application()
    : cameraController(inputManager)
{
    PROFILE_FUNCTION(Magenta); // Profile the constructor

    Logger::Init();
    logger = Logger::GetLogger();
    logger->info("Application started.");
}

Application::~Application() {
    PROFILE_FUNCTION(Magenta); // Profile the destructor

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
    PROFILE_FUNCTION(Magenta); // Profile the Init function

    // Start a profiling block for application initialization
    {
        PROFILE_BLOCK("App Init", Yellow);

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
        {
            PROFILE_BLOCK("Initialize ImGui", Yellow);
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            const char* glsl_version = "#version 460";
            ImGui_ImplOpenGL3_Init(glsl_version);
            ImGui::StyleColorsDark();
            logger->info("ImGui initialized successfully.");
            // Profiling block ends here
        }

        // Initialize TestMenu with shared_ptr
        {
            PROFILE_BLOCK("Initialize TestMenu", Yellow);
            testMenu = std::make_shared<TestMenu>(testManager);
            // Register tests
            testMenu->RegisterTest("Lights", []() { return std::make_shared<TestLights>(); });
            //testMenu->RegisterTest("Compute", []() { return std::make_shared<TestComputeShader>(); });
            //testMenu->RegisterTest("Helmet", []() { return std::make_shared<TestDamagedHelmet>(); });
            testMenu->RegisterTest("Bistro", []() { return std::make_shared<TestBistro>(); });
            testMenu->RegisterTest("Flipbook", []() { return std::make_shared<TestFlipBookEffect>(); });

            // Register TestMenuTest with a weak_ptr to TestMenu
            testManager.RegisterTest("Test Menu", [this]() {
                return std::make_shared<TestMenuTest>(testMenu);
                });
            testManager.SwitchTest("Test Menu");
            // Profiling block ends here
        }

        // Create GPU queries
        {
            PROFILE_BLOCK("Create GPU Queries", Yellow);
            GLCall(glGenQueries(1, &g_queryStart));
            GLCall(glGenQueries(1, &g_queryEnd));
            // Profiling block ends here
        }

        // Profiling block ends here
    }
}

void Application::Run()
{
    PROFILE_FUNCTION(Cyan); // Profile the Run function

    if (!window) {
        logger->error("Cannot run application without a valid window.");
        return;
    }

    lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // Start profiling the frame
        {
            PROFILE_BLOCK("Frame", Purple);

            // Start CPU timer
            auto cpuStart = std::chrono::high_resolution_clock::now();

            // Start GPU timer query
            GLCall(glBeginQuery(GL_TIME_ELAPSED, g_queryStart));

            // Clear the framebuffer
            {
                PROFILE_BLOCK("Render Setup", Blue);
                GLCall(glClearColor(0.3f, 0.2f, 0.8f, 1.0f));
                GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                // Profiling block ends here
            }

            // Update time
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // Handle input
            {
                PROFILE_BLOCK("Handle Input", Blue);
                glfwPollEvents();
                ProcessInput(deltaTime);
                // Profiling block ends here
            }

            // Update and render current test
            {
                PROFILE_BLOCK("Update and Render Test", Blue);
                testManager.UpdateCurrentTest(static_cast<float>(deltaTime));
                testManager.RenderCurrentTest();
                UpdateCameraController();
                // Profiling block ends here
            }

            // IMGUI rendering
            {
                PROFILE_BLOCK("ImGui Rendering", Blue);
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                testManager.RenderCurrentTestImGui();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                // Profiling block ends here
            }

            // End GPU query
            GLCall(glEndQuery(GL_TIME_ELAPSED));

            // Ensure all commands completed before reading the query
            GLCall(glFinish());

            // Fetch GPU timing
            GLuint64 gpuTime = 0;
            GLCall(glGetQueryObjectui64v(g_queryStart, GL_QUERY_RESULT, &gpuTime));
            double gpuFrameTimeMs = static_cast<double>(gpuTime) / 1000000.0; // ns to ms

            // Swap buffers and update input
            {
                PROFILE_BLOCK("Swap Buffers", Blue);
                glfwSwapBuffers(window);
                inputManager.Update();
                // Profiling block ends here
            }

            // Stop CPU timer
            auto cpuEnd = std::chrono::high_resolution_clock::now();
            double cpuFrameTimeMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();

            // Show FPS and times in title
            {
                PROFILE_BLOCK("Update FPS and Times", Blue);
                showFPSAndTimes(window, cpuFrameTimeMs, gpuFrameTimeMs);
                // Profiling block ends here
            }

            // Profiling block ends here
        }
    }

    // Cleanup
    {
        PROFILE_BLOCK("Cleanup", Magenta);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        GLContext::Cleanup(window);
        // Profiling block ends here
    }
}

void Application::ProcessInput(double deltaTime) {
    PROFILE_FUNCTION(Yellow); // Profile the ProcessInput function

    cameraController.Update(deltaTime);
    if (inputManager.WasKeyJustPressed(GLFW_KEY_R)) {
        logger->info("Reloading all shaders...");
        ShaderManager::GetInstance().ReloadAllShaders();
    }
}

void Application::UpdateCameraController() {
    PROFILE_FUNCTION(Yellow); // Profile the UpdateCameraController function

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
    PROFILE_FUNCTION(Green); // Profile the SetupCallbacks function

    // Key Callback
    {
        PROFILE_BLOCK("Setup Key Callback", Yellow);
        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            {
                PROFILE_BLOCK("Key Callback Execution", Yellow);
                auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, GLFW_TRUE);

                bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
                app->inputManager.SetKey(key, isPressed);
                // Profiling block ends here
            }
            });
        // Profiling block ends here
    }

    // Cursor Position Callback
    {
        PROFILE_BLOCK("Setup Cursor Position Callback", Yellow);
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            {
                PROFILE_BLOCK("Cursor Position Callback Execution", Yellow);
                auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    app->cameraController.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
                }
                else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    app->cameraController.Reset();
                }
                // Profiling block ends here
            }
            });
        // Profiling block ends here
    }

    // Scroll Callback
    {
        PROFILE_BLOCK("Setup Scroll Callback", Yellow);
        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            {
                PROFILE_BLOCK("Scroll Callback Execution", Yellow);
                auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
                app->cameraController.ProcessMouseScroll(static_cast<float>(yoffset));
                // Profiling block ends here
            }
            });
        // Profiling block ends here
    }

    // Window Size Callback
    {
        PROFILE_BLOCK("Setup Window Size Callback", Yellow);
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            {
                PROFILE_BLOCK("Window Size Callback Execution", Yellow);
                auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
                if (height == 0)
                    height = 1;

                glViewport(0, 0, width, height);
                Screen::SetResolution(width, height);

                app->testManager.HandleWindowResize(width, height);
                app->cameraController.UpdateFOV();
                // Profiling block ends here
            }
            });
        // Profiling block ends here
    }
}