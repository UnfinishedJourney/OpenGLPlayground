#include "Renderer/Renderer.h"
#include "Utilities/Logger.h"
#include "Scene/FrameData.h"
#include "Utilities/Utility.h" 
#include "Graphics/Buffers/UniformBuffer.h"

Renderer::Renderer()
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    m_ResourceManager = std::make_unique<ResourceManager>();
    logger->info("ResourceManager initialized successfully.");

    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), 0, GL_DYNAMIC_DRAW);
    logger->info("Created FrameData UBO with binding point 0.");

    // Initialize lights data with one light
    m_LightsData = {
        { glm::vec4(10.0f, 10.0f, 10.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
        { glm::vec4(10.0f, 10.0f, 10.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) },
        { glm::vec4(10.0f, 10.0f, 10.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) }
        // Add more lights here if needed
    };

    //Calculate buffer size: size of numLights + size of lightsData
    GLsizeiptr bufferSize = sizeof(glm::vec4) + m_LightsData.size() * sizeof(LightData); // need to understand why std430 layout becomes std140, so uint needs to use 16 bytes, maybe because perframedata uses 140
    //GLsizeiptr bufferSize = m_LightsData.size() * sizeof(LightData);

    auto a = sizeof(uint32_t);
    auto b = sizeof(glm::vec4);

    // Create and bind the SSBO for lights
    glGenBuffers(1, &m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW); // Allocate without initializing

    // Initialize numLights and lightsData
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), &numLights);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), m_LightsData.size() * sizeof(LightData), m_LightsData.data());

    //glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_LightsData.size() * sizeof(LightData), m_LightsData.data());

    // Bind the SSBO to binding point 1 (ensure your shaders use the same binding point)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    logger->info("Initialized Lights SSBO with {} light(s).", m_LightsData.size());
}

Renderer::~Renderer()
{
    glDeleteBuffers(1, &m_LightsSSBO);
    // Delete other OpenGL resources if needed
}

void Renderer::UpdateLightsData(const std::vector<LightData>& lights) const
{
    //will add when will start changing scene lights
}

void Renderer::UpdateFrameDataUBO() const
{
    FrameCommonData frameData;
    frameData.view = FrameData::s_View;  
    frameData.proj = FrameData::s_Projection; 
    frameData.cameraPos = glm::vec4(FrameData::s_CameraPos, 1.0f);

    m_FrameDataUBO->SetData(frameData);
}

void Renderer::BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const
{
    m_ResourceManager->BindShader(shaderName);
    m_ResourceManager->BindMaterial(materialName);
}

void Renderer::Render(const std::shared_ptr<RenderObject>& renderObject) const
{
    //UpdateFrameDataUBO();
    ////UpdateLightsData(m_LightsData);
    //BindShaderAndMaterial(renderObject->GetShaderName(), renderObject->GetMaterialName());

    //glm::mat4 modelMatrix = renderObject->GetTransform()->GetModelMatrix();
    //glm::mat3 normalMatrix = renderObject->GetTransform()->GetNormalMatrix();
    //glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * modelMatrix;

    //m_ResourceManager->SetUniform("u_MVP", mvp);
    //m_ResourceManager->SetUniform("u_Model", modelMatrix);
    //m_ResourceManager->SetUniform("u_NormalMatrix", normalMatrix);

    //renderObject->GetMeshBuffer()->Bind();
    //GLCall(glDrawElements(GL_TRIANGLES, renderObject->GetMeshBuffer()->GetIndexCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const
{
    if (!meshBuffer) {
        Logger::GetLogger()->error("Skybox MeshBuffer is nullptr.");
        return;
    }

    UpdateFrameDataUBO();

    m_ResourceManager->BindShader(shaderName);
    m_ResourceManager->BindCubeMapTexture(textureName, 0);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View;
    m_ResourceManager->SetUniform("u_MVP", mvp);

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
        // Bind shader and material
        BindShaderAndMaterial(batch->GetShaderName(), batch->GetMaterialName());

        glm::mat4 modelMatrix = batch->GetTransform()->GetModelMatrix();
        glm::mat3 normalMatrix = batch->GetTransform()->GetNormalMatrix();

        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * modelMatrix;
        auto t = batch->GetTransform();
        m_ResourceManager->SetUniform("u_MVP", mvp);
        m_ResourceManager->SetUniform("u_Model", modelMatrix);
        m_ResourceManager->SetUniform("u_NormalMatrix", normalMatrix);
        // Render the batch
        batch->Render();
    }
}

void Renderer::Clear() const
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}