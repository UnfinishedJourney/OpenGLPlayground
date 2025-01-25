#include "Renderer/Passes/SkyBoxPass.h"
#include "Scene/Scene.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Resources/ShaderManager.h"
#include "Resources/TextureManager.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"

#include <glad/glad.h>

SkyBoxPass::SkyBoxPass(std::shared_ptr<FrameBuffer> framebuffer,
    const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{

    InitializeSceneResources(scene);
}

void SkyBoxPass::InitializeSceneResources(const std::shared_ptr<Scene>& scene)
{
    // We'll request a simple cube MeshBuffer from MeshManager
    // We'll only need the positions (no normals, no UVs), so set the layout accordingly:
    MeshLayout skyBoxMeshLayout = {
        /* positions  */ true,
        /* normals   */ false,
        /* texCoords */ false,
        /* tangents  */ false,
        /* boneData  */ {}
    };

    auto& meshManager = MeshManager::GetInstance();
    m_SkyboxMeshBuffer = meshManager.GetMeshBuffer("cube", skyBoxMeshLayout);
    if (!m_SkyboxMeshBuffer)
    {
        Logger::GetLogger()->error("Failed to acquire skybox cube mesh buffer.");
    }
}

void SkyBoxPass::Execute(const std::shared_ptr<Scene>& scene)
{
    // Bind our target framebuffer (could be the main framebuffer or an off-screen one)
    m_Framebuffer->Bind();

    // Bind the skybox shader
    auto& shaderManager = ShaderManager::GetInstance();
    auto shader = shaderManager.GetShader("skyBox");
    if (!shader)
    {
        Logger::GetLogger()->error("SkyBox shader not found.");
        return;
    }
    shader->Bind();

    // Bind the scene�s uniform buffer (camera matrices, etc.)
    // The skybox vertex shader will remove translation inside the shader code itself.
    scene->BindFrameDataUBO();

    // Bind the cubemap texture (assume it is named "pisaCube" in your TextureManager)
    auto pisaCube = TextureManager::GetInstance().GetTexture("overcastSky");
    if (!pisaCube)
    {
        Logger::GetLogger()->error("SkyBox cubemap 'pisaCube' not found.");
        return;
    }
    pisaCube->Bind(0);

    // Adjust depth states so the skybox is drawn behind all other geometry
    glDepthMask(GL_FALSE);     // Don�t write to depth
    glDepthFunc(GL_LEQUAL);    // Pass if depth <= current depth

    // Draw the cube
    m_SkyboxMeshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_SkyboxMeshBuffer->GetIndexCount()),
        GL_UNSIGNED_INT,
        nullptr));
    m_SkyboxMeshBuffer->Unbind();

    // Restore normal depth states
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    // Unbind
    m_Framebuffer->Unbind();
}

void SkyBoxPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}

SkyBoxPass::~SkyBoxPass()
{
    // Cleanup if needed
}