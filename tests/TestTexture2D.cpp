#include "TestTexture2D.h"
#include "Renderer.h"
#include "imgui.h"

namespace test {
	TestTexture2D::TestTexture2D()
        : m_TranslationA(200, 200, 0), m_TranslationB(400, 200, 0)
	{
        float positions[] = {
         -50.0f, -50.0f, 0.0f, 0.0f, // 0
         50.0f, -50.0f, 1.0f, 0.0f, // 1
         50.0f, 50.0f, 1.0f, 1.0f, // 2
         -50.0f, 50.0f, 0.0f, 1.0f  // 3
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        m_Shader = std::make_unique<Shader>("../shaders/Basic.shader");
        m_Texture = std::make_unique<Texture>("../assets/cute_dog.png");

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        m_VB = std::make_unique<VertexBuffer>(positions, 4 * 4 * sizeof(float));
        VertexBufferLayout layout;

        layout.Push<float>(2);
        layout.Push<float>(2);
        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(*m_VB, layout);
        m_IB = std::make_unique<IndexBuffer>(indices, 6);

        m_Proj = glm::ortho(0.0f, (float)Screen::s_Width, 0.0f, (float)Screen::s_Height, -1.0f, 1.0f);
        m_View = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0.0, 0.0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200.0, 0.0));

        m_Shader->Bind();
        m_Texture->Bind();
	}

	TestTexture2D::~TestTexture2D()
	{

	}

	void TestTexture2D::OnUpdate(float deltaTime)
	{

	}

	void TestTexture2D::OnRender()
	{
		GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

        Renderer renderer;
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationA);
            glm::mat4 mvp = m_Proj * m_View * model;
            m_Shader->Bind();
            m_Shader->SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(*m_VAO, *m_IB, *m_Shader);
        }

        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationB);
            glm::mat4 mvp = m_Proj * m_View * model;
            m_Shader->Bind();
            m_Shader->SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(*m_VAO, *m_IB, *m_Shader);
        }
	}

	void TestTexture2D::OnImGuiRender()
	{
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        ImGui::SliderFloat3("TranslationA", &m_TranslationA.x, 0.0f, 960.0f);
        ImGui::SliderFloat3("TranslationB", &m_TranslationB.x, 0.0f, 960.0f);
        ImGui::End();
	}
}