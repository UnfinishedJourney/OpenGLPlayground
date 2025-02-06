#include "Renderer/Passes/DebugLightsPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>
#include "Graphics/Shaders/Shader.h"

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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_Framebuffer->Bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //scene->BindLightSSBO();

    scene->UpdateFrameDataUBO();
    scene->BindFrameDataUBO();
    auto lightManager = scene->GetLightManager();
    lightManager->BindLightsGPU();

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
        static_cast<GLsizei>(lightManager->GetLightsData().size())
    ));

    glDisable(GL_CULL_FACE);

    m_LightSphereMeshBuffer->Unbind();
    m_Framebuffer->Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DebugLightsPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

DebugLightsPass::~DebugLightsPass()
{
}