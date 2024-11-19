#pragma once

#include <memory>
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Shaders/Shader.h"
#include "Resources/ResourceManager.h"

class PostProcessingEffect {
public:
    PostProcessingEffect(std::shared_ptr<MeshBuffer> quad, int width = 800, int height = 600)
        : m_FullscreenQuadMeshBuffer(quad), m_Width(width), m_Height(height)
    {};
    virtual ~PostProcessingEffect() = default;

    virtual void Apply(GLuint inputTexture, GLuint outputFramebuffer) = 0;
    virtual void OnWindowResize(int width, int height) = 0;
    virtual void SetParameters(const std::unordered_map<std::string, float>& params) = 0;

protected:
    std::shared_ptr<MeshBuffer> m_FullscreenQuadMeshBuffer;
    int m_Width, m_Height;
    std::shared_ptr<Shader> m_Shader;
};