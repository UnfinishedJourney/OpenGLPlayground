#include "TestSkyBox.h"
#include <GLFW/glfw3.h>
#include "Renderer/Renderer.h"
#include "imgui.h"

namespace test {
    TestSkyBox::TestSkyBox()
    {
        //GLCall(glDisable(GL_DEPTH_TEST));
        SkyBoxHelper sbh;
        auto texID = sbh.LoadCubeMap("../assets/cube/pisa/pisa", ".png");
        m_SkyBox = std::make_unique<SkyBox>(texID, 100.0);
        m_Shader = std::make_unique<Shader>("../shaders/SkyBox.shader");
        m_Shader->Bind();
    }

    TestSkyBox::~TestSkyBox()
    {

    }

    void TestSkyBox::OnUpdate(float deltaTime)
    {
    }

    void TestSkyBox::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        m_SkyBox->Draw(m_Shader);
    }

    void TestSkyBox::OnImGuiRender()
    {
    }
}