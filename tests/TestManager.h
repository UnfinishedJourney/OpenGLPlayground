#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "Test.h"
#include "Scene/Camera.h"

/**
 * @class TestManager
 * @brief Manages the currently active Test and a registry of all available Tests.
 */
class TestManager
{
public:
    /**
     * @brief Constructs a new TestManager.
     */
    TestManager();

    /**
     * @brief Destructs the TestManager, ensuring the current test is properly exited.
     */
    ~TestManager();

    /**
     * @brief Registers a new test creation function with a given name.
     *
     * @param name       Name of the test (key in the registry).
     * @param createFunc Lambda/function that creates a shared_ptr<Test>.
     */
    void RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc);

    /**
     * @brief Switches the current test to the one identified by 'testName'.
     *
     * @param testName Name of the desired test in the registry.
     */
    void SwitchTest(const std::string& testName);

    /**
     * @brief Updates the currently active test.
     *
     * @param deltaTime Time elapsed (in seconds) since last frame.
     */
    void UpdateCurrentTest(float deltaTime);

    /**
     * @brief Renders the currently active test.
     */
    void RenderCurrentTest();

    /**
     * @brief Renders the ImGui GUI elements of the currently active test.
     */
    void RenderCurrentTestImGui();

    /**
     * @brief Forwards window resize events to the currently active test.
     *
     * @param width  New width of the window.
     * @param height New height of the window.
     */
    void HandleWindowResize(int width, int height);

    /**
     * @brief Retrieves the camera associated with the current test (if any).
     *
     * @return A shared pointer to the test’s camera, or nullptr if none.
     */
    std::shared_ptr<Camera> GetCurrentCamera() const;

private:
    /**
     * @brief Registry mapping test names to a creation lambda.
     */
    std::unordered_map<std::string, std::function<std::shared_ptr<Test>()>> m_TestRegistry;

    /**
     * @brief Pointer to the currently active test.
     */
    std::shared_ptr<Test> m_CurrentTest;
};