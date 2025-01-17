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

using MaterialID = int;

/**
 * @brief UniformValue holds any parameter type we might send to the shader.
 */
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

/**
 * @brief A Material in the engine, storing standard parameters (ambient, diffuse, etc.)
 *        plus standard textures (albedo, normal, etc.) and custom ones.
 */
class Material {
public:
    // Constructors / destructor
    Material(MaterialLayout layout)
        : m_Layout(layout)
    {}

    ~Material() = default;

    // Name and ID
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }

    void SetID(MaterialID id) { m_ID = id; }
    MaterialID GetID() const { return m_ID; }

    // Layout
    const MaterialLayout& GetLayout() const { return m_Layout; }

    // Set standard params and custom params
    void SetParam(MaterialParamType type, const UniformValue& value);
    bool GetParam(MaterialParamType type, UniformValue& outValue) const;

    void SetCustomParam(const std::string& name, const UniformValue& value);
    bool GetCustomParam(const std::string& name, UniformValue& outValue) const;

    // Textures
    void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
    std::shared_ptr<ITexture> GetTexture(TextureType type) const;

    void SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture);
    std::shared_ptr<ITexture> GetCustomTexture(const std::string& name) const;

    // Binding
    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

private:
    MaterialID m_ID = -1;             // Unique ID for faster lookups
    std::string m_Name;               // Also keep a name for clarity
    const MaterialLayout m_Layout;          // Which param/textures we hold

    // Storage
    std::unordered_map<MaterialParamType, UniformValue>         m_Params;
    std::unordered_map<std::string, UniformValue>               m_CustomParams;
    std::unordered_map<TextureType, std::shared_ptr<ITexture>>  m_Textures;
    std::unordered_map<std::string, std::shared_ptr<ITexture>>  m_CustomTextures;
};