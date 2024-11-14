#include "DebugLightsPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
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

    m_LightSphereMeshBuffer = resourceManager.GetMeshBuffer("lightsphere", lightMeshLayout);
}

void DebugLightsPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    scene->BindLightSSBO();
    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.BindShader("debugLights");
    m_LightSphereMeshBuffer->Bind();

    GLCall(glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_LightSphereMeshBuffer->GetIndexCount()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(scene->GetLights().size())));

    m_Framebuffer->Unbind();
}

void DebugLightsPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

DebugLightsPass::~DebugLightsPass()
{
}