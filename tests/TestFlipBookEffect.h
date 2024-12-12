#pragma once
#include "Test.h"
#include "Graphics/Effects/EffectsManager.h"
#include <GLFW/glfw3.h>

class TestFlipBookEffect : public Test {
public:
    TestFlipBookEffect() : Test() {}

    void OnEnter() override;

    void OnUpdate(float deltaTime) override;

    void OnRender() override;

    void OnImGuiRender() override;

private:
    std::shared_ptr<FlipbookEffect> m_Flipbook;
};