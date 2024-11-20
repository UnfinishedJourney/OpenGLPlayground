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
        auto& meshManager = MeshManager::GetInstance();
        auto& textureManager = TextureManager::GetInstance();
        auto& materialManager = MaterialManager::GetInstance();

        // Get mesh
        auto cubeMesh = meshManager.GetMesh("cube");
        if (!cubeMesh) {
            Logger::GetLogger()->error("Failed to get mesh 'cube'");
            return;
        }

        MeshLayout cubeMeshLayout = {
            true, // Positions
            false, // Normals
            false, // Texture Coordinates
            false, // Tangents and Bitangents
            {TextureType::Albedo}
        };

        //auto cubeMeshBuffer = meshManager.GetMeshBuffer("cube", cubeMeshLayout);

        // Get texture
        auto texture = textureManager.GetTexture2D("cuteDog");
        if (!texture) {
            Logger::GetLogger()->error("Failed to load texture 'cuteDog'");
            return;
        }

        // Create material
        auto material = std::make_shared<Material>();
        material->AddTexture(texture); // Assuming your shader uses 'u_Texture' uniform
        materialManager.AddMaterial("cubeMaterial", material);

        auto transform = std::make_shared<Transform>();

        // Create render object
        auto m_Cube = std::make_shared<RenderObject>(cubeMesh, cubeMeshLayout, "cubeMaterial", "basic", transform);
        m_Scene->AddRenderObject(m_Cube);

        // Build batches
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