#pragma once

#include <memory>
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Shaders/Shader.h"
#include "Resources/ResourceManager.h"

class PostProcessingEffect {
public:
    virtual ~PostProcessingEffect() = default;

    virtual void Initialize() = 0;
    virtual void Apply(GLuint inputTexture, GLuint outputFramebuffer) = 0;
    virtual void OnWindowResize(int width, int height) = 0;

protected:
    // Fullscreen quad mesh buffer
    std::shared_ptr<MeshBuffer> m_FullscreenQuadMeshBuffer;
    // Shader used by the effect
    std::shared_ptr<Shader> m_Shader;

    // Initialize the fullscreen quad mesh buffer
    void SetupFullscreenQuad();
};