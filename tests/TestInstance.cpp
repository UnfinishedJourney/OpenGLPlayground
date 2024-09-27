#include "TestInstance.h"
#include "Cube.h"
#include "InstanceBuffer.h"

test::TestInstance::TestInstance()
{
    MeshLayout cubeMeshLayout = {
        true,
        false,
        false,
        false,
        true
    };

    std::shared_ptr<MeshBuffer> meshComponent = s_ResourceManager->GetMeshBuffer("cube", cubeMeshLayout);
    std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("instance");
    std::shared_ptr<Texture> texture = s_ResourceManager->GetTexture("cuteDog");

    std::shared_ptr<Material> material = std::make_shared<Material>();
    std::unique_ptr<Transform> transform = std::make_unique<Transform>();
    transform->SetRotation(glm::vec4(0.5, 1.0, 0.0, 0.5));

    material->AddTexture(texture);
    material->SetShader(shader);

    m_Instances = std::make_unique<InstancedRenderObject>(meshComponent, material, std::move(transform));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

test::TestInstance::~TestInstance()
{
}

void test::TestInstance::OnUpdate(float deltaTime)
{
}

void test::TestInstance::OnRender()
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    {
        m_Instances->Render();
    }
}

void test::TestInstance::OnImGuiRender()
{
}
