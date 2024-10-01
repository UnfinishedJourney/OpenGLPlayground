#include "TestAssimp.h"
#include "Renderer/Renderer.h"
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
        auto duckModel = s_ResourceManager->GetModel("duck");
     
        MeshLayout duckMeshLayout = {
            true,
            true,
            false,
            false,
            true
        };

        std::shared_ptr<MeshBuffer> duckMeshBuffer = duckModel->GetMeshBuffer(0, duckMeshLayout);

        std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("duck");
        std::shared_ptr<Texture2D> texture = s_ResourceManager->GetTexture("duckDiffuse");

        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::unique_ptr<Transform> transform = std::make_unique<Transform>();
        transform->SetRotation(glm::vec3(-1.57, 0.0, 0.0));

        material->AddTexture(texture);
        material->SetShader(shader);

        m_DuckRenderObject = std::make_unique<MovingDuck>(duckMeshBuffer, material, std::move(transform));
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
        m_DuckRenderObject->Render();
    }

    void TestAssimp::OnImGuiRender()
    {
    }
}