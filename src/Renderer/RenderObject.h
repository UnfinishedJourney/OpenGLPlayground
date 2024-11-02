#pragma once

#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Scene/Transform.h"

#include <memory>
#include <utility>
#include <string>


struct MaterialName {
    std::string value;

    explicit MaterialName(std::string name) : value(std::move(name)) {}
};

struct ShaderName {
    std::string value;

    explicit ShaderName(std::string name) : value(std::move(name)) {}
};

inline MaterialName operator"" _mt(const char* str, std::size_t size) {
    return MaterialName{ std::string(str) };
}

inline ShaderName operator"" _sh(const char* str, std::size_t size) {
    return ShaderName{ std::string(str) };
}

class MeshBuffer;

class RenderObject {
public:
    RenderObject(std::shared_ptr<MeshBuffer> meshBuffer, MaterialName materialName, ShaderName shaderName, std::shared_ptr<Transform> transform)
        : m_MaterialName(std::move(materialName.value)),
        m_ShaderName(std::move(shaderName.value)),
        m_MeshBuffer(std::move(meshBuffer)),
        m_Transform(std::move(transform))
    {
    }

    virtual ~RenderObject() = default;

    RenderObject(const RenderObject&) = delete;

    RenderObject& operator=(const RenderObject&) = delete;

    RenderObject(RenderObject&&) noexcept = default;

    RenderObject& operator=(RenderObject&&) noexcept = default;

    virtual void Update(float deltaTime)
    {
    }
    std::string m_MaterialName;
    std::string m_ShaderName;
    std::shared_ptr<MeshBuffer> m_MeshBuffer;
    std::shared_ptr<Transform> m_Transform;
protected:
};