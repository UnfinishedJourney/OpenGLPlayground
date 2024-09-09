#include "Test3D.h"
#include "Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

namespace test {
    Test3D::Test3D()
    {

        m_Cube = std::make_unique<Cube>();
        m_Shader = std::make_unique<Shader>("../shaders/Basic.shader");
        m_Shader->Bind();
    }

    Test3D::~Test3D()
    {

    }

    void Test3D::OnUpdate(float deltaTime)
    {
        m_Cube->Update(deltaTime);
    }

    void Test3D::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        Renderer renderer;
        {
            m_Model = glm::rotate(m_Model, (float)glfwGetTime()*0.001f, glm::vec3(0.5f, 1.0f, 0.0f));
            glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Model;
            m_Shader->Bind();
            m_Shader->SetUniformMat4f("u_MVP", mvp);
            m_Cube->Draw(m_Shader);
        }
    }

    void Test3D::OnImGuiRender()
    {
    }
}