#include "TestSimpleCube.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include <GLFW/glfw3.h>
#include "Scene/Screen.h"

namespace test {

    TestSimpleCube::TestSimpleCube()
    {
    }

    void TestSimpleCube::OnEnter()
    {
        auto& resourceManager = ResourceManager::GetInstance();

        auto cubeMesh = resourceManager.GetMesh("cube");
        MeshLayout cubeMeshLayout = {
            true, // Positions
            false, // Normals
            false, // Texture Coordinates
            false, // Tangents and Bitangents
            {TextureType::Albedo}
        };

        // Use the new GetTexture2D method
        auto texture = resourceManager.GetTexture2D("cuteDog");

        auto material = std::make_shared<Material>();
        material->AddTexture(texture);
        resourceManager.AddMaterial("cubeMaterial", material);

        auto transform = std::make_shared<Transform>();

        auto m_Cube = std::make_shared<RenderObject>(cubeMesh, cubeMeshLayout, "cubeMaterial", "basic", transform);
        m_Scene->AddRenderObject(m_Cube);
        m_Scene->BuildBatches();
    }

    void TestSimpleCube::OnExit()
    {
        m_Renderer.reset();
        m_Scene->Clear();
    }

    void TestSimpleCube::OnUpdate(float deltaTime)
    {
        // Update logic if needed
    }

    void TestSimpleCube::OnRender()
    {
        if (m_Renderer && m_Scene)
        {
            m_Renderer->RenderScene(m_Scene);
        }
    }

    void TestSimpleCube::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}