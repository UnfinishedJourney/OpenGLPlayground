#include "GeometryPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    InitializeSceneResources(scene);
}

void GeometryPass::InitializeSceneResources(const std::shared_ptr<Scene>& scene)
{
    // No additional resources needed for now
}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Update LODs based on camera distance
    scene->UpdateLOD(3);

    scene->UpdateFrameDataUBO();
    scene->BuildBatches();
    scene->BindFrameDataUBO();
    scene->BindLightSSBO();

    const auto& batches = scene->GetBatches();
    for (const auto& batch : batches) {
        const auto& renderObjects = batch->GetRenderObjects();
        if (renderObjects.empty()) {
            continue;
        }

        auto& resourceManager = ResourceManager::GetInstance();
        auto& shaderManager = ShaderManager::GetInstance();
        auto& materialManager = MaterialManager::GetInstance();

        auto shader = shaderManager.GetShader(batch->GetShaderName());
        if (shader) {
            shader->Bind();
        }
        else {
            Logger::GetLogger()->error("Shader '{}' not found.", batch->GetShaderName());
            continue;
        }

        materialManager.BindMaterial(batch->GetMaterialName(), shader);

        // Assuming all render objects in the batch have the same model matrix
        glm::mat4 modelMatrix = renderObjects.front()->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = renderObjects.front()->GetTransform()->GetNormalMatrix();

        shader->SetUniform("u_Model", modelMatrix);
        shader->SetUniform("u_NormalMatrix", normalMatrix);

        batch->Render();
    }

    m_Framebuffer->Unbind();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

GeometryPass::~GeometryPass()
{
}