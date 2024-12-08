#include "TestComputeShader.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Graphics/Effects/EffectsManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestComputeShader::TestComputeShader() {
    // Initialization if needed
}

void TestComputeShader::OnEnter() {

    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();
    auto& textureManager = TextureManager::GetInstance();

    // Add light to the scene
    LightData light = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f) , glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    m_Scene->AddLight(light);

    light = { glm::vec4(-1.5f, 2.0f, -1.5f, 0.0f) , glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
    m_Scene->AddLight(light);

    m_Scene->SetPostProcessingEffect(PostProcessingEffectType::PresentTexture);

    auto tex = textureManager.GetTexture2D("heightmap");
    std::unordered_map<std::string, EffectParameter> params;
    params.insert({ "texture", tex });
    auto& effectsManager = EffectsManager::GetInstance();
    effectsManager.SetEffectParameters(PostProcessingEffectType::PresentTexture, params);
}

void TestComputeShader::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestComputeShader::OnUpdate(float deltaTime) {
    // Update objects or animations if needed
}


void TestComputeShader::OnImGuiRender() {
    ImGui::Begin("TestComputeShader Controls");
    ImGui::End();
}
