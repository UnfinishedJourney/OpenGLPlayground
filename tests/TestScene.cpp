#include "TestScene.h"
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
    TestScene::TestScene()
    {
        m_Scene = std::make_unique<Scene>();
    }

    TestScene::~TestScene()
    {

    }

    void TestScene::OnUpdate(float deltaTime)
    {

    }

    void TestScene::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        m_Scene->Draw();
    }

    void TestScene::OnImGuiRender()
    {
    }
}