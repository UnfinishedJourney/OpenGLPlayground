#include "TestFlipBookEffect.h"

#include "Graphics/Effects/EffectsManager.h"
#include <GLFW/glfw3.h>

void TestFlipBookEffect::OnEnter() {
    auto& effectsManager = EffectsManager::GetInstance();
    m_Flipbook = effectsManager.GetFlipbookEffect("FireBall04"); // loads from config if not present
}

void TestFlipBookEffect::OnUpdate(float deltaTime) {
    if (m_Flipbook) {
        double currentTime = glfwGetTime();
        m_Flipbook->Update(currentTime);
    }
}

void TestFlipBookEffect::OnRender() {
    // Render scene first
    Test::OnRender();

    // Render flipbook effects after scene
    if (m_Flipbook) {
        m_Flipbook->Render();
    }
}

void TestFlipBookEffect::OnMouseClick(float x, float y) {
    int width = Screen::s_Width;
    int height = Screen::s_Height;
    float mx = (x / (float)width) * 2.0f - 1.0f;
    float my = (y / (float)height) * 2.0f - 1.0f;

    if (m_Flipbook) {
        m_Flipbook->SpawnAnimation({ mx, my }, 0);
    }
}