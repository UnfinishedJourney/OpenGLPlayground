#include "GeometryPass.h"
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    InitializeSceneResources(scene);
}

GeometryPass::~GeometryPass() {}

void GeometryPass::InitializeSceneResources(const std::shared_ptr<Scene>& scene)
{
    // Possibly create additional buffers or GPU resources
}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Example: LOD updates (distance-based)
    scene->UpdateLODs();

    // Build the scene’s batches
    scene->UpdateFrameDataUBO();
    scene->BuildBatches();

    // Bind UBO/SSBO
    scene->BindFrameDataUBO();
    scene->BindLightSSBO();

    const auto& batches = scene->GetBatches();
    for (const auto& batch : batches) {
        const auto& ros = batch->GetRenderObjects();
        if (ros.empty()) continue;

        auto& shaderManager = ShaderManager::GetInstance();
        auto& materialManager = MaterialManager::GetInstance();

        auto shader = shaderManager.GetShader(batch->GetShaderName());
        if (!shader) {
            Logger::GetLogger()->error("Shader '{}' not found.", batch->GetShaderName());
            continue;
        }
        shader->Bind();

        materialManager.BindMaterial(batch->GetMaterialName(), shader);

        // For geometry pass, each batch uses the same transform only if the ROs share it 
        // but in practice, they could differ. If your code lumps them together, you do one matrix.
        // But multi-draw indirect means per-draw transform. For now, let's do the first:
        glm::mat4 modelMatrix = ros.front()->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = ros.front()->GetTransform()->GetNormalMatrix();

        shader->SetUniform("u_Model", modelMatrix);
        shader->SetUniform("u_NormalMatrix", normalMatrix);

        batch->Render();
    }

    m_Framebuffer->Unbind();
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}