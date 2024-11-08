#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include "Scene/FrameData.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>

#define DEBUG_LIGHTS

Renderer::Renderer()
    : m_FrameDataUBO(nullptr), m_LightsSSBO(0)
{
    // Constructor is private; initialization is done in Initialize()
}

Renderer::~Renderer()
{
    glDeleteBuffers(1, &m_LightsSSBO);
}

void Renderer::Initialize()
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), 0, GL_DYNAMIC_DRAW);
    logger->info("Created FrameData UBO with binding point 0.");

    // Initialize lights data
    m_LightsData = {
        { glm::vec4(1.5f, 1.5f, 1.5f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
        // Add more lights as needed
    };

    // Calculate buffer size
    GLsizeiptr bufferSize = sizeof(glm::vec4) + m_LightsData.size() * sizeof(LightData);

    // Create and bind the SSBO for lights
    glGenBuffers(1, &m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

    // Initialize numLights and lightsData
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), &numLights);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), m_LightsData.size() * sizeof(LightData), m_LightsData.data());

    // Bind the SSBO to binding point 1
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    logger->info("Initialized Lights SSBO with {} light(s).", m_LightsData.size());

    auto& resourceManager = ResourceManager::GetInstance();

    MeshLayout lightMeshLayout = {
        true, 
        false, 
        false, 
        false, 
        {}
    };

    m_LightSphereMeshBuffer = resourceManager.GetMeshBuffer("lightsphere", lightMeshLayout);

}

void Renderer::UpdateLightsData(const std::vector<LightData>& lights) const
{
    // Update lights data as needed
}

void Renderer::UpdateFrameDataUBO() const
{
    FrameCommonData frameData;
    frameData.view = FrameData::s_View;
    frameData.proj = FrameData::s_Projection;
    frameData.cameraPos = glm::vec4(FrameData::s_CameraPos, 1.0f);

    m_FrameDataUBO->SetData(&frameData, sizeof(FrameCommonData));
}

void Renderer::BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const
{
    ResourceManager::GetInstance().BindShader(shaderName);
    ResourceManager::GetInstance().BindMaterial(materialName);
}

void Renderer::RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const
{
    if (!meshBuffer) {
        Logger::GetLogger()->error("Skybox MeshBuffer is nullptr.");
        return;
    }

    UpdateFrameDataUBO();

    ResourceManager::GetInstance().BindShader(shaderName);
    ResourceManager::GetInstance().BindCubeMapTexture(textureName, 0);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 mvp = FrameData::s_Projection * glm::mat4(glm::mat3(FrameData::s_View)); // Remove translation
    ResourceManager::GetInstance().SetUniform("u_MVP", mvp);

    meshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(meshBuffer->GetIndexCount()), GL_UNSIGNED_INT, nullptr));

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void Renderer::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_BatchManager.AddRenderObject(renderObject);
}

void Renderer::RenderScene()
{
    UpdateFrameDataUBO();
    m_BatchManager.BuildBatches();

    const auto& batches = m_BatchManager.GetBatches();
    for (const auto& batch : batches) {

        // Get the model matrix from the first RenderObject
        const auto& renderObjects = batch->GetRenderObjects();
        if (renderObjects.empty()) {
            continue; // Skip if no render objects
        }

        // Bind shader and material
        BindShaderAndMaterial(batch->GetShaderName(), batch->GetMaterialName());

        glm::mat4 modelMatrix = renderObjects.front()->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = renderObjects.front()->GetTransform()->GetNormalMatrix();

        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * modelMatrix;
        ResourceManager::GetInstance().SetUniform("u_MVP", mvp);
        ResourceManager::GetInstance().SetUniform("u_Model", modelMatrix);
        ResourceManager::GetInstance().SetUniform("u_NormalMatrix", normalMatrix);

        // Render the batch
        batch->Render();
    }

#ifdef DEBUG_LIGHTS
    RenderLightSpheres();
#endif
}

void Renderer::RenderLightSpheres()
{
    //if (!m_LightSphereShader || !m_LightSphereMesh) {
    //    Logger::GetLogger()->error("Light spheres shader or mesh is not initialized.");
    //    return;
    //}

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.BindShader("debugLights");

    // Bind the sphere mesh
    m_LightSphereMeshBuffer->Bind();

    // Draw all instances in a single draw call
    GLCall(glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_LightSphereMeshBuffer->GetIndexCount()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(m_LightsData.size())));
}

void Renderer::ClearRenderObjects()
{
    m_BatchManager.Clear();
}

void Renderer::Clear(float r, float g, float b, float a) const
{
    GLCall(glClearColor(r, g, b, a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}