#include "GeometryPass.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include "Utilities/ProfilerMacros.h"

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer,
    const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    // Potentially store references or do setup
}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    {
        PROFILE_BLOCK("Bind Framebuffer", Magenta);
        m_Framebuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // 1) Build static batches if needed
    {
        PROFILE_BLOCK("Build Static Batches", Yellow);
        scene->BuildStaticBatchesIfNeeded();
    }

    // 2) Cull & LOD
    {
        PROFILE_BLOCK("Cull and LOD Update", Yellow);
        scene->CullAndLODUpdate();
    }

    // 3) Update frame data + bind
    {
        PROFILE_BLOCK("Update and Bind UBOs", Yellow);
        scene->UpdateFrameDataUBO();
        scene->BindFrameDataUBO();
        scene->BindLightSSBO();
    }

    // 4) Render batches
    {
        PROFILE_BLOCK("Render Static Batches", Cyan);
        auto& materialManager = MaterialManager::GetInstance();
        auto& shaderManager = ShaderManager::GetInstance();

        const auto& staticBatches = scene->GetStaticBatches();
        for (auto& batch : staticBatches)
        {
            PROFILE_BLOCK("Render Batch", Purple);
            if (batch->GetRenderObjects().empty()) continue;

            // Grab shader
            auto shader = shaderManager.GetShader(batch->GetShaderName());
            if (!shader) {
                Logger::GetLogger()->error("Shader '{}' not found.", batch->GetShaderName());
                continue;
            }
            shader->Bind();

            // Material
            materialManager.BindMaterial(batch->GetMaterialName(), shader);

            // Model matrix from first object if all merged
            auto roTransform = batch->GetTransform();
            glm::mat4 modelMatrix = roTransform.GetModelMatrix();
            glm::mat3 normalMatrix = roTransform.GetNormalMatrix();

            shader->SetUniform("u_Model", modelMatrix);
            shader->SetUniform("u_NormalMatrix", normalMatrix);

            // Actually render
            batch->Render();

            // Unbind material if needed
            materialManager.UnbindMaterial();
        }
    }

    {
        PROFILE_BLOCK("Unbind Framebuffer", Magenta);
        m_Framebuffer->Unbind();
    }
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    PROFILE_FUNCTION(Green);
    m_Framebuffer = framebuffer;
}