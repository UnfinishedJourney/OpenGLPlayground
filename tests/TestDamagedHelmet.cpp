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
    std::unordered_map<aiTextureType, TextureType> aiToMyType = {
      { aiTextureType_DIFFUSE,  TextureType::Albedo      },
      { aiTextureType_NORMALS,  TextureType::Normal      },
      { aiTextureType_LIGHTMAP, TextureType::AO          },
      { aiTextureType_UNKNOWN,  TextureType::MetalRoughness },
      { aiTextureType_EMISSIVE, TextureType::Emissive    }
    };

    if (!m_Scene->LoadStaticModelIntoScene("helmet", "helmetPBR", 1.0, aiToMyType)) {
        Logger::GetLogger()->error("Failed to load 'helmet' model in TestDamagedHelmet");
        return;
    }

    m_Scene->SetSkyboxEnabled(true);
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