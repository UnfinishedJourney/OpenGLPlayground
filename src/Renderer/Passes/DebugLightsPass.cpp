#include "Renderer/Passes/DebugLightsPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>
#include "Graphics/Shaders/Shader.h"

DebugLightsPass::DebugLightsPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
    const std::shared_ptr<Scene::Scene>& scene)
    : framebuffer_(framebuffer)
{
    InitializeSceneResources(scene);
}

void DebugLightsPass::InitializeSceneResources(const std::shared_ptr<Scene::Scene>& scene) {
    auto& meshManager = graphics::MeshManager::GetInstance();
    MeshLayout lightMeshLayout = { true, false, false, false, {} };
    lightSphereMeshBuffer_ = meshManager.GetMeshBuffer("lightsphere", lightMeshLayout);
}

void DebugLightsPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    framebuffer_->Bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    scene->UpdateFrameDataUBO();
    scene->BindFrameDataUBO();
    auto lightManager = scene->GetLightManager();
    lightManager->BindLightsGPU();

    auto shader = graphics::ShaderManager::GetInstance().GetShader("debugLights");
    if (!shader) {
        Logger::GetLogger()->error("DebugLightsPass: 'debugLights' shader not found.");
        return;
    }
    shader->Bind();
    lightSphereMeshBuffer_->Bind();

    GLCall(glDrawElementsInstanced(GL_TRIANGLES,
        static_cast<GLsizei>(lightSphereMeshBuffer_->GetIndexCount()),
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(lightManager->GetLightsData().size())));

    glDisable(GL_CULL_FACE);
    lightSphereMeshBuffer_->Unbind();
    framebuffer_->Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DebugLightsPass::UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) {
    framebuffer_ = framebuffer;
}

DebugLightsPass::~DebugLightsPass() {
    // Resources will be released by smart pointers.
}