#include "TestLights.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"

namespace test {

    TestLights::TestLights()
    {
    }

    void TestLights::OnEnter()
    {
        m_Scene = std::make_shared<Scene>();

        auto& resourceManager = ResourceManager::GetInstance();

        // Create and set the camera
        auto camera = std::make_shared<Camera>();
        m_Scene->SetCamera(camera);

        // Load meshes and materials
        MeshLayout pigMeshLayout = {
            true,  // Positions
            true,  // Normals
            false, // Texture Coordinates
            false, // Tangents and Bitangents
            {}
        };

        auto meshInfos = resourceManager.GetModelMeshInfos("pig");
        auto material = std::make_shared<Material>();

        // Gold material properties
        material->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
        material->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
        material->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
        material->AddParam<float>("material.shininess", 51.2f);

        resourceManager.AddMaterial("pigMaterial", material);

        auto transform = std::make_shared<Transform>();

        // Add render objects to the scene
        for (auto& [meshTextures, mesh] : meshInfos)
        {
            auto ro = std::make_shared<RenderObject>(mesh, pigMeshLayout, "pigMaterial", "simplelights", transform);
            m_Scene->AddRenderObject(ro);
        }

        // Add lights to the scene
        LightData light = { glm::vec4(1.5f, 1.5f, 1.5f, 0.0f) , glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
        m_Scene->AddLight(light);

        // Build batches
        m_Scene->BuildBatches();

        // Enable blending if needed
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void TestLights::OnExit()
    {
        m_Scene->Clear();
    }

    void TestLights::OnUpdate(float deltaTime)
    {
        // Update objects or animations if needed
    }

    void TestLights::OnRender()
    {
        Renderer::GetInstance().RenderScene(m_Scene);
    }

    void TestLights::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}