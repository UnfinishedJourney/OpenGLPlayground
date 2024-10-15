#include "TestSimpleCube.h"

test::TestSimpleCube::TestSimpleCube(std::shared_ptr<Renderer>& renderer)
    : Test(renderer)
{
    //std::shared_ptr<Mesh> cubeMesh = s_ResourceManager->GetMesh("cube");
    MeshLayout cubeMeshLayout = {
        true,
        false,
        false,
        false,
        {TextureType::Albedo}
    };

    std::shared_ptr<MeshBuffer> meshComponent = m_Renderer->m_ResourceManager->GetMeshBuffer("cube", cubeMeshLayout);
    std::shared_ptr<Shader> shader = m_Renderer->m_ResourceManager->GetShader("basic");
    std::shared_ptr<Texture2D> texture = m_Renderer->m_ResourceManager->GetTexture("cuteDog");

    std::shared_ptr<Material> material = std::make_shared<Material>();
    std::unique_ptr<Transform> transform = std::make_unique<Transform>();

    material->AddTexture(texture);
    m_Renderer->m_ResourceManager->AddMaterial("dogMat", material);

    m_Cube = std::make_unique<MovingCube>(meshComponent, "dogMat"_mt, "basic"_sh, std::move(transform));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void test::TestSimpleCube::OnUpdate(float deltaTime)
{
    m_Cube->Update(deltaTime);
}

void test::TestSimpleCube::OnRender()
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    m_Renderer->Render(m_Cube);
}

void test::TestSimpleCube::OnImGuiRender()
{
}
