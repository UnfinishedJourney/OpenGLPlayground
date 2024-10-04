#include "TestSimpleCube.h"
#include "Graphics/Meshes/Cube.h"

test::TestSimpleCube::TestSimpleCube()
{
    //std::shared_ptr<Mesh> cubeMesh = s_ResourceManager->GetMesh("cube");
    MeshLayout cubeMeshLayout = {
        true,
        false,
        false,
        false,
        {TextureType::Albedo}
    };

    std::shared_ptr<MeshBuffer> meshComponent = s_ResourceManager->GetMeshBuffer("cube", cubeMeshLayout);
    std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("basic");
    std::shared_ptr<Texture2D> texture = s_ResourceManager->GetTexture("cuteDog");

    std::shared_ptr<Material> material = std::make_shared<Material>();
    std::unique_ptr<Transform> transform = std::make_unique<Transform>();
    //transform->SetRotation(glm::vec4(0.5, 1.0, 0.0, 0.5));

    material->AddTexture(texture);
    material->SetShader(shader);

    m_Cube = std::make_unique<MovingCube>(meshComponent, material, std::move(transform));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

test::TestSimpleCube::~TestSimpleCube()
{
}

void test::TestSimpleCube::OnUpdate(float deltaTime)
{
    m_Cube->Update(deltaTime);
}

void test::TestSimpleCube::OnRender()
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    {
        m_Cube->Render();
    }
}

void test::TestSimpleCube::OnImGuiRender()
{
}
