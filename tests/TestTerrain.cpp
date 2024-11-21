#include "TestTerrain.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include <GLFW/glfw3.h>
#include "Scene/Screen.h"

namespace test {

    TestTerrain::TestTerrain()
    {
    }

    void TestTerrain::OnEnter()
    {
        auto& textureManager = TextureManager::GetInstance();

        auto heightmap = textureManager.GetTexture2D("heightmap");
        if (!heightmap) {
            Logger::GetLogger()->error("Failed to load texture 'heightmap'");
            return;
        }
        m_Scene->SetTerrainHeightMap(heightmap);

        //m_Scene->BuildBatches();
    }

    void TestTerrain::OnExit()
    {
        m_Renderer.reset();
        m_Scene->Clear();
    }

    void TestTerrain::OnUpdate(float deltaTime)
    {
        // Update logic if needed
    }

    void TestTerrain::OnRender()
    {
        if (m_Renderer && m_Scene)
        {
            m_Renderer->RenderScene(m_Scene);
        }
    }

    void TestTerrain::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}