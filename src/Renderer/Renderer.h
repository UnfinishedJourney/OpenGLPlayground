#pragma once

#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Utility.h"

#include <glad/glad.h>

#include <memory>

class ResourceManager;

class Renderer
{
public:
    Renderer();
    void Render(const RenderObject& renderObject) const;
    void Clear() const;
private:
    std::unique_ptr<ResourceManager> m_ResourceManager;
};
