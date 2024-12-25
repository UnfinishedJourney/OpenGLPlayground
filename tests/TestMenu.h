#pragma once

#include <vector>
#include <functional>
#include <string>
#include "TestManager.h"

class TestMenu {
public:
    explicit TestMenu(TestManager& testManager);

    void OnImGuiRender();
    void RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc);

private:
    TestManager& m_TestManager;
    std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
};

class TestMenuTest : public Test {
public:
    explicit TestMenuTest(std::weak_ptr<TestMenu> testMenu);
    ~TestMenuTest() override;
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnImGuiRender() override;
    void OnWindowResize(int width, int height) override;
    std::shared_ptr<Camera> GetCamera() const override;

private:
    std::weak_ptr<TestMenu> m_TestMenu;
};