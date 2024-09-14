#include "TestAssimp.h"
#include "Renderer.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <ext.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <numeric>

namespace test {
    TestAssimp::TestAssimp()
    {
        m_ResourceManager = std::make_unique<ResourceManager>();
        m_DuckModel = m_ResourceManager->GetModel("duck");
        
        std::shared_ptr<Mesh> duckMesh = m_DuckModel->getMesh(0);

        MeshLayout duckMeshLayout = {
            true,
            true,
            false,
            false,
            true
        };

        MeshHelper meshHelper;

        std::shared_ptr<MeshComponent> meshComponent = meshHelper.CreateMeshComponent(duckMesh, duckMeshLayout);
        std::shared_ptr<Shader> shader = std::make_shared<Shader>("../shaders/Duck.shader");
        std::shared_ptr<Texture> texture = std::make_shared<Texture>("../assets/rubber_duck/textures/Duck_baseColor.png");

        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::unique_ptr<Transform> transform = std::make_unique<Transform>();

        material->AddTexture(texture);
        material->SetShader(shader);

        m_DuckRenderObject = std::make_unique<MovingDuck>(meshComponent, material, std::move(transform));
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    }

    TestAssimp::~TestAssimp()
    {

    }

    void TestAssimp::OnUpdate(float deltaTime)
    {
        m_DuckRenderObject->Update(deltaTime);
    }

    void TestAssimp::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        m_DuckRenderObject->Draw();
    }

    void TestAssimp::OnImGuiRender()
    {
    }
}