#include "TestSimpleCube.h"
#include "Cube.h"

test::TestSimpleCube::TestSimpleCube()
{
    std::shared_ptr<Mesh> cubeMesh = std::make_shared<Cube>();
    MeshLayout cubeMeshLayout = {
        true,
        false,
        false,
        false,
        true
    };

    MeshHelper meshHelper;

    std::shared_ptr<MeshComponent> meshComponent = meshHelper.CreateMeshComponent(cubeMesh, cubeMeshLayout);
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("../shaders/Basic.shader");
    std::shared_ptr<Texture> texture = std::make_shared<Texture>("../assets/cute_dog.png");

    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->AddTexture(texture);
    material->SetShader(shader);

    glm::mat4 modelMatrix = glm::mat4(1.0);
    m_Cube = std::make_unique<RenderObject>(meshComponent, material, modelMatrix);
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

}

test::TestSimpleCube::~TestSimpleCube()
{
}

void test::TestSimpleCube::OnUpdate(float deltaTime)
{
}

void test::TestSimpleCube::OnRender()
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    {
        m_Cube->Draw();
    }
}

void test::TestSimpleCube::OnImGuiRender()
{
}
