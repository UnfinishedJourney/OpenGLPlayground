#include "TestLights.h"
#include "Utilities/Logger.h"

#include <imgui.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

TestLights::TestLights()
    : Test()
{
}

void TestLights::OnEnter()
{
    // Load a model into the scene
    if (!scene_->LoadStaticModelIntoScene("pig", "simplelights")) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestLights");
        return;
    }

    // Add a light
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 1.0f), glm::vec4(1.0f) };

    auto lightManager = scene_->GetLightManager();
    lightManager->AddLight(light1);

    // Enable debug lights and grid
    scene_->SetShowGrid(true);
    scene_->SetShowDebugLights(true);
    auto& materials = graphics::MaterialManager::GetInstance().GetMaterials();
    if (materials.size() > 0) {
        materials[0]->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.9f, 0.1f, 0.3f));
    }
}

void TestLights::OnExit()
{
    // Cleanup
    renderer_.reset();
    scene_->Clear();
}

void TestLights::OnUpdate(float /*deltaTime*/)
{
    // LOD or culling
    scene_->CullAndLODUpdate();
}

void TestLights::OnImGuiRender()
{
    ImGui::Begin("TestLights Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}