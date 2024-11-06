#pragma once

#include <memory>
#include <vector>
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Renderer/RenderObject.h"
#include "Renderer/BatchManager.h"
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
    static Renderer& GetInstance() {
        static Renderer instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Initialize(); // Added initialization method
    void RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const;
    void UpdateLightsData(const std::vector<LightData>& lights) const;
    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    void RenderScene();

private:
    Renderer();
    ~Renderer();

    void UpdateFrameDataUBO() const;
    void BindShaderAndMaterial(const std::string& shaderName, const std::string& materialName) const;

    std::unique_ptr<UniformBuffer> m_FrameDataUBO;
    GLuint m_LightsSSBO;

    std::vector<LightData> m_LightsData;

    BatchManager m_BatchManager;
};