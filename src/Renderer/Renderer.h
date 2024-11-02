#pragma once

#include <memory>
#include "Resources/ResourceManager.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Lights.h"

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

private:
    void UpdateFrameDataUBO() const;

    void BindShaderAndMaterial(const std::shared_ptr<RenderObject>& renderObject) const;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    GLuint m_LightsSSBO;

    // Assuming you have a collection of lights
    std::vector<LightData> m_LightsData;
};