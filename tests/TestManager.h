#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "Test.h"
#include "Scene/Camera.h"

class TestManager {
public:
    TestManager();
    ~TestManager();

    void RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc);
    void SwitchTest(const std::string& testName);
    void UpdateCurrentTest(float deltaTime);
    void RenderCurrentTest();
    void RenderCurrentTestImGui();
    void HandleWindowResize(int width, int height);
    std::shared_ptr<Camera> GetCurrentCamera() const;

private:
    std::unordered_map<std::string, std::function<std::shared_ptr<Test>()>> m_TestRegistry;
    std::shared_ptr<Test> m_CurrentTest;
};