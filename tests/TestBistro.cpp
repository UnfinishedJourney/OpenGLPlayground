#include "TestBistro.h"
#include "Scene/Scene.h"
#include "Scene/Scene.h"
#include "Graphics/Materials/MaterialManager.h"
#include "Scene/Lights.h"
#include "Utilities/Logger.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

TestBistro::TestBistro() : Test() {}

void TestBistro::OnEnter() {
    // Initialize Camera
    //m_Camera = std::make_shared<Camera>(
    //    glm::vec3(10.0f, 5.0f, 15.0f), // Position: further away for a large scene
    //    glm::vec3(0.0f, 1.0f, 0.0f),   // Up vector
    //    -135.0f,                        // Yaw: facing towards the origin
    //    -20.0f                          // Pitch: looking slightly downward
    //);
    //m_Camera->SetSpeed(200.0f);
    //m_Camera->SetFarPlane(10000.0f);
    //scene_->SetCamera(m_Camera);

    // Load the "bistro" model into the scene

    auto camera = GetCamera();
    camera->SetFarPlane(250.0);
    camera->SetNearPlane(1.0);
    //if (!scene_->LoadModelIntoScene("bistro", "basicTextured", "objMaterial", 0.01)) {
    //if (!scene_->LoadModelIntoScene("bistro", "bistroShader", "bistroMaterial", 0.01)) {
    //    Logger::GetLogger()->error("Failed to load 'bistro' model into the scene.");
    //    return;
    //}

    if (!scene_->LoadStaticModelIntoScene("bistroExterior", "bistroShaderShadowed", 0.01)) {
        Logger::GetLogger()->error("Failed to load 'bistroExterior' model in TestBistro");
        return;
    }

    //if (!scene_->LoadStaticModelIntoScene("bistroInterior", "bistroShaderShadowed", 0.01)) {
    //    Logger::GetLogger()->error("Failed to load 'bistroInterior' model in TestBistro");
    //    return;
    //}

    glm::vec3 lDir(-0.1f, -1.0f, 0.0f);
    lDir = glm::normalize(lDir);
    LightData light1 = { glm::vec4(lDir, 0.0f), glm::vec4(1.0f) };

    auto lightManager = scene_->GetLightManager();
    lightManager->AddLight(light1);

    scene_->SetShowShadows(true);
    scene_->SetShadowMapSize(1024 * 8);
    scene_->BuildStaticBatchesIfNeeded();

    scene_->SetSkyboxEnabled(true);

    // Initialize Materials
    //auto& materialManager = MaterialManager::GetInstance();
    //if (!materialManager.GetMaterial("objMaterial")) {
    //    // Create a default material if not already present
    //    glm::vec3 ambient(0.2f, 0.2f, 0.2f);
    //    glm::vec3 diffuse(0.5f, 0.5f, 0.5f);
    //    glm::vec3 specular(1.0f, 1.0f, 1.0f);
    //    float shininess = 32.0f;

    //    auto defaultMaterial = materialManager.CreateMaterial("objMaterial", ambient, diffuse, specular, shininess);
    //    materialManager.AddMaterial("objMaterial", defaultMaterial);
    //}

}

void TestBistro::OnExit() {
    renderer_.reset();
    scene_->Clear();
}

void TestBistro::OnUpdate(float deltaTime) {
    //scene_->CullAndLODUpdate();
}

void TestBistro::OnImGuiRender() {
    ImGui::Begin("TestBistro Controls");

    //// Camera Controls
    //if (ImGui::CollapsingHeader("Camera")) {
    //    glm::vec3& position = m_Camera->GetPositionRef();
    //    ImGui::SliderFloat3("Position", &position.x, -100.0f, 100.0f);
    //    float& yaw = m_Camera->GetYawRef();
    //    ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f);
    //    float& pitch = m_Camera->GetPitchRef();
    //    ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f);
    //    m_Camera->RecalculateViewMatrix();
    //}

    //// Lighting Controls
    //if (ImGui::CollapsingHeader("Lights")) {
    //    const auto& lights = scene_->GetLightsData();
    //    for (size_t i = 0; i < lights.size(); ++i) {
    //        std::string header = "Light " + std::to_string(i + 1);
    //        if (ImGui::TreeNode(header.c_str())) {
    //            // Position/Direction
    //            glm::vec4& posDir = const_cast<glm::vec4&>(lights[i].positionDirection);
    //            ImGui::DragFloat3("Position/Direction", &posDir.x, 0.1f);

    //            // Color
    //            glm::vec4& color = const_cast<glm::vec4&>(lights[i].color);
    //            ImGui::ColorEdit3("Color", &color.x);

    //            ImGui::TreePop();
    //        }
    //    }

    //    if (ImGui::Button("Add Point Light")) {
    //        LightData newPointLight = {
    //            glm::vec4(0.0f, 5.0f, 0.0f, 1.0f), // Position
    //            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)  // Color
    //        };
    //        scene_->AddLight(newPointLight);
    //    }

    //    if (ImGui::Button("Add Directional Light")) {
    //        LightData newDirLight = {
    //            glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f), // Direction
    //            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)      // Color
    //        };
    //        scene_->AddLight(newDirLight);
    //    }
    //}

    //// Grid and Debug Lights
    //if (ImGui::CollapsingHeader("Scene Settings")) {
    //    bool grid = scene_->GetBGrid();
    //    if (ImGui::Checkbox("Show Grid", &grid)) {
    //        scene_->SetBGrid(grid);
    //    }

    //    bool debugLights = scene_->GetBDebugLights();
    //    if (ImGui::Checkbox("Debug Lights", &debugLights)) {
    //        scene_->SetBDebugLights(debugLights);
    //    }

    //    // Post-Processing Effects
    //    PostProcessingEffectType effect = scene_->GetPostProcessingEffect();
    //    const char* effects[] = { "None", "Edge Detection", "Bloom", "Motion Blur" };
    //    int currentEffect = static_cast<int>(effect);
    //    if (ImGui::Combo("Post-Processing Effect", &currentEffect, effects, IM_ARRAYSIZE(effects))) {
    //        scene_->SetPostProcessingEffect(static_cast<PostProcessingEffectType>(currentEffect));
    //    }
    //}

    ImGui::End();
}