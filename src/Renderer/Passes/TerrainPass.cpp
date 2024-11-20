#include "TerrainPass.h"
#include "Resources/MeshManager.h"
#include "Resources/ShaderManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

TerrainPass::TerrainPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    auto& meshManager = MeshManager::GetInstance();
    MeshLayout terrainMeshLayout = {
        true,  // Positions
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Albedo } // Texture Coordinates
    };
    m_TerrainMeshBuffer = meshManager.GetMeshBuffer("terrain", terrainMeshLayout);
}

void TerrainPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();

    // Enable necessary OpenGL states
    glEnable(GL_DEPTH_TEST);

    auto& shaderManager = ShaderManager::GetInstance();
    auto shader = shaderManager.GetShader("terrain");
    if (shader) {
        shader->Bind();
    }
    else {
        Logger::GetLogger()->error("Terrain shader not found.");
        return;
    }

    // Bind height map texture
    auto heightMap = scene->GetTerrainHeightMap();
    if (!heightMap) {
        Logger::GetLogger()->error("Height map not set in scene.");
        return;
    }
    heightMap->Bind(0);
    shader->SetUniform("u_HeightMap", 0);

    // Bind frame data UBO
    scene->BindFrameDataUBO();

    // Set model matrix
    glm::mat4 modelMatrix = glm::mat4(1.0f); // Adjust as needed
    shader->SetUniform("u_Model", modelMatrix);

    // Bind the terrain mesh
    m_TerrainMeshBuffer->Bind();

    // Set up tessellation parameters
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Draw the terrain mesh using patches
    if (m_TerrainMeshBuffer->GetIndexCount()) {
        glDrawElements(GL_PATCHES, m_TerrainMeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    }
    else {
        glDrawArrays(GL_PATCHES, 0, m_TerrainMeshBuffer->GetVertexCount());
    }

    m_TerrainMeshBuffer->Unbind();

    m_Framebuffer->Unbind();
}

TerrainPass::~TerrainPass()
{
}

void TerrainPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}