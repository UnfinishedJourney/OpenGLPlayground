#include "Renderer/Passes/DebugLightsPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

DebugLightsPass::DebugLightsPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    InitializeSceneResources(scene);
}

void DebugLightsPass::InitializeSceneResources(const std::shared_ptr<Scene>& scene)
{
    auto& resourceManager = ResourceManager::GetInstance();

    MeshLayout lightMeshLayout = {
        true,
        false,
        false,
        false,
        {}
    };

    auto& meshManager = MeshManager::GetInstance();
    m_LightSphereMeshBuffer = meshManager.GetMeshBuffer("lightsphere", lightMeshLayout);
}

void DebugLightsPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    scene->BindLightSSBO();
    auto& resourceManager = ResourceManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();
    auto shader = shaderManager.GetShader("debugLights");
    if (shader) {
        shader->Bind();
    }
    else {
        Logger::GetLogger()->error("DebugLights shader not found.");
        return;
    }
    m_LightSphereMeshBuffer->Bind();

    GLCall(glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(m_LightSphereMeshBuffer->GetIndexCount()),
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(scene->GetLights().size())
    ));

    m_LightSphereMeshBuffer->Unbind();
    m_Framebuffer->Unbind();
}

void DebugLightsPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

DebugLightsPass::~DebugLightsPass()
{
}