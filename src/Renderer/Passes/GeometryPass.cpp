#include "GeometryPass.h"
#include "Renderer/Batch.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Graphics/Materials/MaterialManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include "Utilities/ProfilerMacros.h"
#include "Graphics/Shaders/Shader.h"

GeometryPass::GeometryPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
    const std::shared_ptr<Scene::Scene>& scene)
    : framebuffer_(framebuffer)
{
    shadowed_ = scene->GetShowShadows();
    if (shadowed_) {
        auto lightManager = scene->GetLightManager();
        glm::mat4 bias = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f);
        glm::mat4 lightView = lightManager->ComputeLightView(0);
        glm::mat4 lightProj = lightManager->ComputeLightProj(0);
        shadowMatrix_ = bias * lightProj * lightView;
    }
}

void GeometryPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    PROFILE_FUNCTION(Blue);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        PROFILE_BLOCK("Bind Framebuffer", Magenta);
        framebuffer_->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    {
        PROFILE_BLOCK("Build Static Batches", Yellow);
        scene->BuildStaticBatchesIfNeeded();
    }

    {
        PROFILE_BLOCK("Cull and LOD Update", Yellow);
        scene->CullAndLODUpdate();
    }

    {
        PROFILE_BLOCK("Update and Bind UBOs", Yellow);
        scene->UpdateFrameDataUBO();
        scene->BindFrameDataUBO();
        auto lightManager = scene->GetLightManager();
        lightManager->BindLightsGPU();
    }

    {
        PROFILE_BLOCK("Render Static Batches", Cyan);
        auto& materialManager = graphics::MaterialManager::GetInstance();
        auto& shaderManager = graphics::ShaderManager::GetInstance();
        const auto& staticBatches = scene->GetStaticBatches();

        for (auto& batch : staticBatches) {
            PROFILE_BLOCK("Render Batch", Purple);
            if (batch->GetRenderObjects().empty())
                continue;

            auto shader = shaderManager.GetShader(batch->GetShaderName());
            if (!shader) {
                Logger::GetLogger()->error("GeometryPass: Shader '{}' not found.", batch->GetShaderName());
                continue;
            }
            shader->Bind();
            materialManager.BindMaterial(batch->GetMaterialID(), shader);
            if (shadowed_) {
                shader->SetUniform("u_ShadowMatrix", shadowMatrix_);
            }
            batch->Render();
            materialManager.UnbindMaterial();
        }
    }

    {
        PROFILE_BLOCK("Unbind Framebuffer", Magenta);
        framebuffer_->Unbind();
    }
    glDisable(GL_BLEND);
}

void GeometryPass::UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) {
    framebuffer_ = framebuffer;
}