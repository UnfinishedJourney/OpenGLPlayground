#include "TestSimpleCube.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include <GLFW/glfw3.h>

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

        auto shader = resourceManager.GetShader("basic");
        auto texture = resourceManager.GetTexture("cuteDog");

        auto material = std::make_shared<Material>();
        material->AddTexture(texture);
        resourceManager.AddMaterial("cubeMaterial", material);

        auto transform = std::make_shared<Transform>();

        auto m_Cube = std::make_shared<RenderObject>(cubeMesh, cubeMeshLayout, "cubeMaterial", "basic", transform);
        Renderer::GetInstance().AddRenderObject(m_Cube);
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }

    void TestSimpleCube::OnExit()
    {
        Renderer::GetInstance().ClearRenderObjects();
        //m_Cube.reset();
    }

    void TestSimpleCube::OnUpdate(float deltaTime)
    {
        //if (m_Cube)
        //{
        //    m_Cube->GetTransform()->AddRotation(glm::vec3(0.0, deltaTime, 0.0));
        //}
    }

    void TestSimpleCube::OnRender()
    {
        // Remove the clear call here if Application is already clearing the screen
        // If not, you can keep it or adjust as needed
        Renderer::GetInstance().Clear(0.3f, 0.4f, 0.55f, 1.0f);
        Renderer::GetInstance().RenderScene();
    }

    void TestSimpleCube::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}