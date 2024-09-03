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
        m_Model = std::make_unique<Model>("../assets/rubber_duck/scene.gltf");
        //m_Model = std::make_unique<Model>("../assets/backpack/backpack.obj");
        m_Model->ProcessModel();
    }

    TestAssimp::~TestAssimp()
    {

    }

    void TestAssimp::OnUpdate(float deltaTime)
    {

    }

    void TestAssimp::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        m_Model->Draw();
    }

    void TestAssimp::OnImGuiRender()
    {
    }
}