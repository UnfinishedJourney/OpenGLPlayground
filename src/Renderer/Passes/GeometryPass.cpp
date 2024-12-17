#include "GeometryPass.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1) Build static batches if needed
    scene->BuildStaticBatchesIfNeeded();

    // 2) Cull + LOD update
    scene->CullAndLODUpdate();

    // 3) Update global UBO
    scene->UpdateFrameDataUBO();
    scene->BindFrameDataUBO();
    scene->BindLightSSBO();

    // 4) Render static batches
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();

    const auto& staticBatches = scene->GetStaticBatches();
    for (auto& batch : staticBatches) {
        if (batch->GetRenderObjects().empty()) continue;

        auto shader = shaderManager.GetShader(batch->GetShaderName());
        if (!shader) {
            Logger::GetLogger()->error("Shader '{}' not found.", batch->GetShaderName());
            continue;
        }
        shader->Bind();

        materialManager.BindMaterial(batch->GetMaterialName(), shader);

        // If all ROs in the batch share the same transform or are merged,
        // we can use the transform from the first object (for typical static merges).
        glm::mat4 modelMatrix = batch->GetRenderObjects().front()->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = batch->GetRenderObjects().front()->GetTransform()->GetNormalMatrix();

        shader->SetUniform("u_Model", modelMatrix);
        shader->SetUniform("u_NormalMatrix", normalMatrix);

        batch->Render();
    }

    // 5) (Optional) Render dynamic objects with single draws or another pass

    m_Framebuffer->Unbind();
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}