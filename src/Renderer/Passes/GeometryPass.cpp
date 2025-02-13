#include "GeometryPass.h"
#include "Renderer/Batch.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Graphics/Materials/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include "Utilities/ProfilerMacros.h"
#include "Graphics/Shaders/Shader.h"

GeometryPass::GeometryPass(std::shared_ptr<FrameBuffer> framebuffer,
    const std::shared_ptr<Scene::Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    m_bShadowed = scene->GetShowShadows();
    if (m_bShadowed) 
    {
        auto lightManager = scene->GetLightManager();

        glm::mat4 bias = glm::mat4(
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f
        );

        glm::mat4 lightView = lightManager->ComputeLightView(0);
        glm::mat4 lightProj = lightManager->ComputeLightProj(0);

        m_ShadowMatrix = bias * lightProj * lightView;

    }

}

void GeometryPass::Execute(const std::shared_ptr<Scene::Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    {
        PROFILE_BLOCK("Bind Framebuffer", Magenta);
        m_Framebuffer->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
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
        auto lightManager = scene->GetLightManager();
        lightManager->BindLightsGPU();
    }

    


    // 4) Render batches
    {
        PROFILE_BLOCK("Render Static Batches", Cyan);
        auto& materialManager = graphics::MaterialManager::GetInstance();
        auto& shaderManager = graphics::ShaderManager::GetInstance();

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
            materialManager.BindMaterial(batch->GetMaterialID(), shader);

            if (m_bShadowed) {
                shader->SetUniform("u_ShadowMatrix", m_ShadowMatrix);
            }
            // Model matrix from first object if all merged
            //glm::mat4 modelMatrix = glm::mat4(1.0f);
            //glm::mat3 normalMatrix = glm::mat4(1.0f);

            //shader->SetUniform("u_Model", modelMatrix);
            //shader->SetUniform("u_NormalMatrix", normalMatrix);

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