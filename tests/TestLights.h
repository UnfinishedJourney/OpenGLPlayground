#pragma once

#include "Test.h"

class TestLights : public Test {
public:
    TestLights();
    ~TestLights() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
    // OnRender, OnWindowResize, and GetCamera inherited from Test
};