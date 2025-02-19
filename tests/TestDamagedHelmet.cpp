#include "TestDamagedHelmet.h"
#include "Graphics/Textures/TextureManager.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestDamagedHelmet::TestDamagedHelmet() {}

void TestDamagedHelmet::OnEnter() {
    if (!scene_->LoadStaticModelIntoScene("helmet", "helmetPBR", 1.0)) {
        Logger::GetLogger()->error("Failed to load 'helmet' model in TestDamagedHelmet");
        return;
    }
    scene_->SetSkyboxEnabled(true);
    //m_Scene->SetPostProcessingEffect(PostProcessingEffectType::ToneMapping);
}

void TestDamagedHelmet::OnExit() {
    renderer_.reset();
    scene_->Clear();
}

void TestDamagedHelmet::OnUpdate(float deltaTime) {
    
}

void TestDamagedHelmet::OnImGuiRender() {
    ImGui::Begin("TestDamagedHelmet Controls");
    float speed = GetCamera()->GetSpeed();

    // Create a slider for adjusting camera speed between 0.1 and 20.0 units.
    if (ImGui::SliderFloat("Camera Speed", &speed, 0.1f, 20.0f)) {
        GetCamera()->SetSpeed(speed);
    }

    ImGui::End();
}