#include "TestDamagedHelmet.h"
#include "Resources/ModelManager.h"
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
    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();
    auto& textureManager = TextureManager::GetInstance();

    MeshLayout objMeshLayout = {
        true,
        true,
        true,
        false,
        {TextureType::Albedo}
    };

    auto model = modelManager.GetModel("helmet");
    if (!model) {
        Logger::GetLogger()->error("Failed to load model 'helmet'");
        return;
    }

    auto material = std::make_shared<Material>();

    auto brdfLUT = textureManager.GetTexture("brdfLUT");
    auto albedo = model->GetTexture(0, TextureType::Albedo);
    auto normal = model->GetTexture(0, TextureType::Normal);
    auto ao = model->GetTexture(0, TextureType::AO);
    auto mr = model->GetTexture(0, TextureType::MetalRoughness);
    auto emissive = model->GetTexture(0, TextureType::Emissive);

    material->AddTexture(brdfLUT, 0);
    material->AddTexture(albedo, 1);
    material->AddTexture(normal, 2);
    material->AddTexture(mr, 3);
    material->AddTexture(ao, 4);
    material->AddTexture(emissive, 5);

    materialManager.AddMaterial("objMaterial", material);

    auto transform = std::make_shared<Transform>();
    transform->SetPosition({ 0.0f, 0.5f, 0.0f });

    const auto& meshinfos = model->GetMeshesInfo();
    for (const auto& minfo : meshinfos) {
        auto renderObject = std::make_shared<RenderObject>(
            minfo.mesh,
            objMeshLayout,
            "objMaterial",
            "HelmetPBR",
            transform
        );
        m_Scene->AddRenderObject(renderObject);
    }

    LightData light = { {10.0f,10.0f,10.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f} };
    m_Scene->AddLight(light);

    m_Scene->BuildBatches();
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