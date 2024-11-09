#include "TestLights.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include <GLFW/glfw3.h>

namespace test {

    TestLights::TestLights()
    {
    }

    void TestLights::OnEnter()
    {
        auto& resourceManager = ResourceManager::GetInstance();

        MeshLayout pigMeshLayout = {
            true, // Positions
            true, // Normals
            false, // Texture Coordinates
            false, // Tangents and Bitangents
            {}
        };

        auto meshInfos = resourceManager.GetModelMeshInfos("pig");
        auto shader = resourceManager.GetShader("simplelights");

        auto material = std::make_shared<Material>();

        //gold
        material->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
        material->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
        material->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
        material->AddParam<float>("material.shininess", 51.2f);

        resourceManager.AddMaterial("pigMaterial", material);

        auto transform = std::make_shared<Transform>();

        auto& renderer = Renderer::GetInstance();

        for (auto& [meshTextures, mesh] : meshInfos)
        {
            auto ro = std::make_shared<RenderObject>(mesh, pigMeshLayout, "pigMaterial", "simplelights", transform);
            renderer.AddRenderObject(ro);
        }
           
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }

    void TestLights::OnExit()
    {
        Renderer::GetInstance().ClearRenderObjects();
    }

    void TestLights::OnUpdate(float deltaTime)
    {
        //if (m_Cube)
        //{
        //    m_Cube->GetTransform()->AddRotation(glm::vec3(0.0, deltaTime, 0.0));
        //}
    }

    void TestLights::OnRender()
    {
        Renderer::GetInstance().Clear(0.3f, 0.4f, 0.55f, 1.0f);
        Renderer::GetInstance().RenderScene();
    }

    void TestLights::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}