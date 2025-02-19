#include "SkyBoxPass.h"
#include "Scene/Scene.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Graphics/Textures/TextureManager.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include "Graphics/Shaders/Shader.h"
#include <glad/glad.h>

SkyBoxPass::SkyBoxPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
    const std::shared_ptr<Scene::Scene>& scene)
    : framebuffer_(framebuffer)
{
    InitializeSceneResources(scene);
}

void SkyBoxPass::InitializeSceneResources(const std::shared_ptr<Scene::Scene>& scene) {
    MeshLayout skyBoxMeshLayout = { true, false, false, false, {} };
    auto& meshManager = graphics::MeshManager::GetInstance();
    skyboxMeshBuffer_ = meshManager.GetMeshBuffer("cube", skyBoxMeshLayout);
    if (!skyboxMeshBuffer_) {
        Logger::GetLogger()->error("Failed to acquire skybox cube mesh buffer.");
    }
}

void SkyBoxPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    framebuffer_->Bind();

    auto& shaderManager = graphics::ShaderManager::GetInstance();
    auto shader = shaderManager.GetShader("skyBox");
    if (!shader) {
        Logger::GetLogger()->error("SkyBox shader not found.");
        return;
    }
    shader->Bind();

    scene->BindFrameDataUBO();

    // Bind cubemap textures.
    std::string skyBoxName = "currentSkybox";
    auto cubeMap = graphics::TextureManager::GetInstance().GetTexture(skyBoxName);
    if (!cubeMap) {
        Logger::GetLogger()->error("SkyBox cubemap '{}' not found.", skyBoxName);
        return;
    }
    cubeMap->Bind(8);

    skyBoxName = "currentSkybox_irr";
    auto cubeMapIrr = graphics::TextureManager::GetInstance().GetTexture(skyBoxName);
    if (!cubeMapIrr) {
        Logger::GetLogger()->error("SkyBox cubemap '{}' not found.", skyBoxName);
        return;
    }
    cubeMapIrr->Bind(9);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    skyboxMeshBuffer_->Bind();
    GLCall(glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(skyboxMeshBuffer_->GetIndexCount()),
        GL_UNSIGNED_INT,
        nullptr));
    skyboxMeshBuffer_->Unbind();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    framebuffer_->Unbind();
}

void SkyBoxPass::UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) {
    framebuffer_ = framebuffer;
}

SkyBoxPass::~SkyBoxPass() {
    // Cleanup handled by smart pointers.
}
