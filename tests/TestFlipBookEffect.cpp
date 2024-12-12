#include "TestFlipBookEffect.h"

#include "Graphics/Effects/EffectsManager.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

void TestFlipBookEffect::OnEnter() {
    auto& effectsManager = EffectsManager::GetInstance();
    m_Flipbook = effectsManager.GetFlipbookEffect("Flame02"); // loads from config if not present
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

void TestFlipBookEffect::OnImGuiRender() {
    if (ImGui::Button("Spawn FireBall Effect")) {
        // Spawn animation at the center of the screen (0,0)
        if (m_Flipbook) {
            m_Flipbook->SpawnAnimation({ 0.0f, 0.0f }, 0);
        }
    }
}