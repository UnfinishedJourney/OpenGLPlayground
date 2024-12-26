#pragma once

#include <variant>
#include <unordered_map>
#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "MaterialParamType.h"
#include "MaterialLayout.h"
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

/**
 * A Material in the engine, storing standard parameters (ambient, diffuse, etc.)
 * plus standard textures (albedo, normal, etc.) and custom ones.
 */
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

class Material {
public:
    Material() = default;
    ~Material() = default;

    // Name
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }

    // Param
    void SetParam(MaterialParamType type, const UniformValue& value);
    void SetCustomParam(const std::string& name, const UniformValue& value);

    bool GetParam(MaterialParamType type, UniformValue& outValue) const;
    bool GetCustomParam(const std::string& name, UniformValue& outValue) const;

    // Textures
    void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
    void SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture);

    std::shared_ptr<ITexture> GetTexture(TextureType type) const;
    std::shared_ptr<ITexture> GetCustomTexture(const std::string& name) const;

    // Binding
    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

    // Layout
    const MaterialLayout& GetLayout() const { return m_Layout; }

private:
    std::string m_Name;
    MaterialLayout m_Layout;

    // Standard textures
    std::unordered_map<TextureType, std::shared_ptr<ITexture>> m_Textures;

    // Custom textures
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_CustomTextures;

    // Standard params
    std::unordered_map<MaterialParamType, UniformValue> m_Params;

    // Custom params
    std::unordered_map<std::string, UniformValue> m_CustomParams;
};