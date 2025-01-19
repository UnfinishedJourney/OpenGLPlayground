#include "TestDamagedHelmet.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Resources/TextureManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestDamagedHelmet::TestDamagedHelmet() {}

void TestDamagedHelmet::OnEnter() {
  
    if (!m_Scene->LoadModelIntoScene("helmet", "helmetPBR", "helmetMaterial")) {
        Logger::GetLogger()->error("Failed to load 'helmet' model in TestDamagedHelmet");
        return;
    }

    // Add a light
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f), glm::vec4(1.0f) };
    m_Scene->AddLight(light1);

    m_Scene->SetBDebugLights(true);
}

void TestDamagedHelmet::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestDamagedHelmet::OnUpdate(float deltaTime) {}

void TestDamagedHelmet::OnImGuiRender() {
    ImGui::Begin("TestDamagedHelmet Controls");
    ImGui::End();
}