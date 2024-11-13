#include "GeometryPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer), m_LightSSBO(0)
{
    InitializeSceneResources(scene);
}

void GeometryPass::InitializeSceneResources(const std::shared_ptr<Scene>& scene)
{
}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene->UpdateFrameDataUBO();
    scene->BuildBatches();
    scene->BindLightSSBO();
    const auto& batches = scene->GetBatches();
    for (const auto& batch : batches) {

        const auto& renderObjects = batch->GetRenderObjects();
        if (renderObjects.empty()) {
            continue;
        }

        ResourceManager::GetInstance().BindShader(batch->GetShaderName());
        ResourceManager::GetInstance().BindMaterial(batch->GetMaterialName());
        glm::mat4 modelMatrix = renderObjects.front()->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = renderObjects.front()->GetTransform()->GetNormalMatrix();

        ResourceManager::GetInstance().SetUniform("u_Model", modelMatrix);
        ResourceManager::GetInstance().SetUniform("u_NormalMatrix", normalMatrix);

        batch->Render();
    }

    m_Framebuffer->Unbind();
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

GeometryPass::~GeometryPass()
{
}