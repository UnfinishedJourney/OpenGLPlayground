#include "Test3D.h"
#include "Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

namespace test {
    Test3D::Test3D()
    {

        float vertices[] = {
            // Positions         // Texture Coords
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Vertex 0
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // Vertex 1
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // Vertex 2
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // Vertex 3
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // Vertex 4
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // Vertex 5
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // Vertex 6
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f  // Vertex 7
        };

        unsigned int indices[] = {
            // Front face
            0, 1, 2,
            2, 3, 0,

            // Back face
            4, 5, 6,
            6, 7, 4,

            // Left face
            0, 3, 7,
            7, 4, 0,

            // Right face
            1, 5, 6,
            6, 2, 1,

            // Bottom face
            0, 4, 5,
            5, 1, 0,

            // Top face
            3, 2, 6,
            6, 7, 3
        };

        m_Shader = std::make_unique<Shader>("shaders/Basic.shader");
        m_Texture = std::make_unique<Texture>("assets/cute_dog.png");

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        m_VB = std::make_unique<VertexBuffer>(vertices, 5 * 8 * sizeof(float));
        VertexBufferLayout layout;

        layout.Push<float>(3);
        layout.Push<float>(2);
        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(*m_VB, layout);
        m_IB = std::make_unique<IndexBuffer>(indices, 36);

        m_View = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        m_Proj = glm::mat4(1.0f);

        //m_Proj = glm::perspective(glm::radians(45.0f), 960.0f / 540.0f, 0.1f, 100.0f);
        //m_View = glm::translate(m_View, glm::vec3(0.0f, 0.0f, -3.0f));
        m_Model = glm::mat4(1.0f);

        m_Shader->Bind();
        m_Texture->Bind();
    }

    Test3D::~Test3D()
    {

    }

    void Test3D::OnUpdate(float deltaTime)
    {

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
            renderer.Draw(*m_VAO, *m_IB, *m_Shader);
        }
    }

    void Test3D::OnImGuiRender()
    {
    }
}