#include "TestSkyBox.h"
#include "Graphics/Meshes/Cube.h"

test::TestSkyBox::TestSkyBox(std::shared_ptr<Renderer>& renderer)
    : Test(renderer)
{
    MeshLayout skyBoxMeshLayout = {
        true,
        false,
        false,
        false,
        {}
    };

    m_MeshComponent = m_Renderer->m_ResourceManager->GetMeshBuffer("cube", skyBoxMeshLayout);
    std::shared_ptr<Shader> shader = m_Renderer->m_ResourceManager->GetShader("skyBox");
    std::shared_ptr<CubeMapTexture> texture = m_Renderer->m_ResourceManager->GetCubeMapTexture("pisa");

}

void test::TestSkyBox::OnUpdate(float deltaTime)
{
}

void test::TestSkyBox::OnRender()
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    m_Renderer->RenderSkybox(m_MeshComponent, "pisa", "skyBox");
}

void test::TestSkyBox::OnImGuiRender()
{
}
