#include "GridPass.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Resources/MeshManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>
#include "Graphics/Shaders/Shader.h"

GridPass::GridPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene::Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    auto& meshManager = Graphics::MeshManager::GetInstance();
    MeshLayout gridMeshLayout = {
        true,  // Positions
        false, // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };
    m_GridMeshBuffer = meshManager.GetMeshBuffer("quad", gridMeshLayout);
}

void GridPass::Execute(const std::shared_ptr<Scene::Scene>& scene)
{
    m_Framebuffer->Bind();

    // Enable blending for transparency
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set depth function to ensure grid is rendered beneath other objects
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    auto& shaderManager = Graphics::ShaderManager::GetInstance();
    auto shader = shaderManager.GetShader("grid");
    if (shader) {
        shader->Bind();
    }
    else {
        Logger::GetLogger()->error("Grid shader not found.");
        return;
    }

    scene->BindFrameDataUBO();

    // Render the grid mesh
    m_GridMeshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    m_GridMeshBuffer->Unbind();

    // Reset render states
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);

    m_Framebuffer->Unbind();
}

GridPass::~GridPass()
{
}

void GridPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}