#pragma once

#include "Test.h"

class TestDamagedHelmet : public Test {
public:
    TestDamagedHelmet();
    ~TestDamagedHelmet() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
};