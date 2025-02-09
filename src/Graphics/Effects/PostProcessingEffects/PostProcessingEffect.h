#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Shaders/Shader.h"
#include "Resources/ResourceManager.h"


#include <variant>
#include <glm/glm.hpp>
#include "Graphics/Textures/ITexture.h"

using EffectParameter = std::variant<
    float,
    int,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    std::shared_ptr<Graphics::ITexture>
>;

class PostProcessingEffect {
public:
    PostProcessingEffect(std::shared_ptr<Graphics::MeshBuffer> quad, int width = 800, int height = 600)
        : m_FullscreenQuadMeshBuffer(quad), m_Width(width), m_Height(height)
    {}

    virtual ~PostProcessingEffect() = default;

    // Apply the effect using input texture and output framebuffer
    virtual void Apply(GLuint inputTexture, GLuint outputFramebuffer) = 0;

    // Handle window resize events
    virtual void OnWindowResize(int width, int height) = 0;

    // Set parameters using a map of parameter names to EffectParameters
    virtual void SetParameters(const std::unordered_map<std::string, EffectParameter>& params) {};

protected:
    std::shared_ptr<Graphics::MeshBuffer> m_FullscreenQuadMeshBuffer;
    int m_Width, m_Height;
    std::shared_ptr<Shader> m_Shader;
};