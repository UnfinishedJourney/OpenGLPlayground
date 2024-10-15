#pragma once

#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"
#include "Scene/FrameData.h"
#include "Utilities/Utility.h"

#include <glad/glad.h>

#include <memory>

class ResourceManager;

class Renderer
{
public:
    Renderer();
    void Render(const std::shared_ptr<RenderObject>& renderObject) const;
    void RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const;
    void Clear() const;
    std::unique_ptr<ResourceManager> m_ResourceManager;
};
