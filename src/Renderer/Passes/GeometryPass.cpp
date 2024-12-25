#include "GeometryPass.h"
#include "Resources/ShaderManager.h"
#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
}

void GeometryPass::Execute(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    {
        PROFILE_BLOCK("Bind Framebuffer", Magenta);
        m_Framebuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Profiling block ends here
    }

    // 1) Build static batches if needed
    {
        PROFILE_BLOCK("Build Static Batches", Yellow);
        scene->BuildStaticBatchesIfNeeded();
        // Profiling block ends here
    }

    // 2) Cull + LOD update
    {
        PROFILE_BLOCK("Cull and LOD Update", Yellow);
        scene->CullAndLODUpdate();
        // Profiling block ends here
    }

    // 3) Update global UBO
    {
        PROFILE_BLOCK("Update and Bind UBOs", Yellow);
        scene->UpdateFrameDataUBO();
        scene->BindFrameDataUBO();
        scene->BindLightSSBO();
        // Profiling block ends here
    }

    // 4) Render static batches
    {
        PROFILE_BLOCK("Render Static Batches", Cyan);
        auto& materialManager = MaterialManager::GetInstance();
        auto& shaderManager = ShaderManager::GetInstance();

        const auto& staticBatches = scene->GetStaticBatches();
        for (auto& batch : staticBatches) {
            {
                PROFILE_BLOCK("Render Batch", Purple);
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
                // Profiling block ends here
            }
        }
        // Profiling block ends here
    }

    // 5) (Optional) Render dynamic objects with single draws or another pass

    {
        PROFILE_BLOCK("Unbind Framebuffer", Magenta);
        m_Framebuffer->Unbind();
        // Profiling block ends here
    }
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    PROFILE_FUNCTION(Green);
    m_Framebuffer = framebuffer;
}