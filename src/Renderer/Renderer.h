#pragma once

#include <memory>
#include <vector>
#include "Resources/ResourceManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Renderer/RenderObject.h"
#include "Renderer/Batch.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Lights.h"
#include "Renderer/BatchManager.h"

struct FrameCommonData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

class Renderer
{
public:
    Renderer();

    void Render(const std::shared_ptr<RenderObject>& renderObject) const;
    ~Renderer();
    void RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const;
    void UpdateLightsData(const std::vector<LightData>& lights) const;
    void Clear() const;

    std::unique_ptr<ResourceManager> m_ResourceManager;
    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void RenderScene();

private:
    void UpdateFrameDataUBO() const;

    void BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    GLuint m_LightsSSBO;

    // Assuming you have a collection of lights
    std::vector<LightData> m_LightsData;

    std::vector<std::unique_ptr<Batch>> m_Batches;
    BatchManager m_BatchManager;
};