#pragma once

#include <vector>
#include <memory>
#include "Renderer/RenderObject.h"
#include "Renderer/BatchManager.h"
#include "Scene/Lights.h"
#include "Scene/Camera.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"

struct FrameCommonData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

class Scene {
public:
    Scene();
    ~Scene();

    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    //void AddLight(const std::shared_ptr<Light>& light);
    void AddLight(LightData light);
    void SetCamera(const std::shared_ptr<Camera>& camera);
    void Clear();

    // IRenderDataProvider implementations
    const std::vector<std::shared_ptr<Batch>>& GetBatches() const;
    void UpdateFrameDataUBO() const;
    void UpdateLightsData() const;
    void BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const;

    void BuildBatches() const;
    void BindLightSSBO() const;

    // Accessors
    const std::shared_ptr<Camera>& GetCamera() const { return m_Camera; }

private:

    const int MAX_LIGHTS = 8;
    //std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
    //std::vector<std::shared_ptr<Light>> m_Lights;
    std::vector<LightData> m_LightsData;
    std::shared_ptr<Camera> m_Camera;

    mutable BatchManager m_BatchManager;
    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    //std::unique_ptr<ShaderStorageBuffer> m_LightsSSBO;
    GLuint m_LightsSSBO;

    // Other necessary data and methods
};