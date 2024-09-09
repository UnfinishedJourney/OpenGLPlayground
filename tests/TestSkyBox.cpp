#include "TestSkyBox.h"
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "imgui.h"

namespace test {
    TestSkyBox::TestSkyBox()
    {
        //GLCall(glDisable(GL_DEPTH_TEST));
        m_SkyBox = std::make_unique<SkyBox>(100.0);
        m_Shader = std::make_unique<Shader>("../shaders/SkyBox.shader");
        m_Shader->Bind();
        SkyBoxHelper sbh;
        m_TexID = sbh.LoadCubeMap("../assets/cube/pisa/pisa", ".png");
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

        m_SkyBox->Draw(m_Shader, m_TexID);
    }

    void TestSkyBox::OnImGuiRender()
    {
    }
}