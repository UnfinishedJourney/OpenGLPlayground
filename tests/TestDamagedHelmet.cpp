#include "TestDamagedHelmet.h"
#include "Graphics/Textures/TextureManager.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestDamagedHelmet::TestDamagedHelmet() {}

void TestDamagedHelmet::OnEnter() {
    //std::unordered_map<aiTextureType, TextureType> aiToMyType = {
    //  { aiTextureType_DIFFUSE,  TextureType::Diffuse      },
    //  { aiTextureType_NORMALS,  TextureType::Normal      },
    //  { aiTextureType_LIGHTMAP, TextureType::AO          },
    //  { aiTextureType_UNKNOWN,  TextureType::MetalRoughness },
    //  { aiTextureType_EMISSIVE, TextureType::Emissive    },
    //  { aiTextureType_METALNESS, TextureType::Height},
    //  { aiTextureType_DIFFUSE_ROUGHNESS, TextureType::Ambient},
    //};

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

void TestDamagedHelmet::OnUpdate(float deltaTime) {}

void TestDamagedHelmet::OnImGuiRender() {
    ImGui::Begin("TestDamagedHelmet Controls");
    ImGui::End();
}