#include "TestShadows.h"
#include "Scene/Scene.h"
#include "Utilities/Logger.h"

#include <imgui.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

TestShadows::TestShadows()
    : Test()
{
}

void TestShadows::OnEnter()
{
    // Configure the scene's camera
    auto cam = std::make_shared<Camera>();
    m_Scene->SetCamera(cam);

    // Load a model into the scene
    if (!m_Scene->LoadStaticModelIntoScene("pig", "simpleLightsShadowed")) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestShadow");
        return;
    }

    if (!m_Scene->LoadPrimitiveIntoScene("floor", "simpleLightsShadowed", 1)) {
        Logger::GetLogger()->error("Failed to load cube primitive.");
    }

    // Add a light
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f), glm::vec4(1.0f) };
    auto lightManager = m_Scene->GetLightManager();
    lightManager->AddLight(light1);

    m_Scene->SetShowDebugLights(true);
    m_Scene->SetShowShadows(true);
    auto existingMat = MaterialManager::GetInstance().GetMaterialByID(1);
    if (existingMat) {
        existingMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.4f, 0.5f, 0.8f));
    }
}

void TestShadows::OnExit()
{
    // Cleanup
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestShadows::OnUpdate(float /*deltaTime*/)
{
    // LOD or culling
    m_Scene->CullAndLODUpdate();
}

void TestShadows::OnImGuiRender()
{
    ImGui::Begin("TestShadow Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}