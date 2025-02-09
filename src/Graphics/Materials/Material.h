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
#include "Graphics/Buffers/UniformBuffer.h"

class UniformBuffer;


/**
 * @brief A variant holding possible uniform parameter types.
 *        For example, floats or vectors used by the material.
 */
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

/**
 * @brief Contains up to four vec4 used for standard MTL parameters:
 *        Ka, Kd, Ks, Ke, Ns, etc. The layout is:
 *
 *        Mtl0 = (Ka.x, Ka.y, Ka.z, Ni)
 *        Mtl1 = (Kd.x, Kd.y, Kd.z, d)
 *        Mtl2 = (Ks.x, Ks.y, Ks.z, Ns)
 *        Mtl3 = (Ke.x, Ke.y, Ke.z, extra)
 */
struct PackedMtlParams
{
    glm::vec4 Mtl0 = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glm::vec4 Mtl1 = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    glm::vec4 Mtl2 = glm::vec4(0.0f, 0.0f, 0.0f, 32.0f);
    glm::vec4 Mtl3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Helper accessors
    inline glm::vec3 Ka() const { return glm::vec3(Mtl0); }
    inline float Ni()  const { return Mtl0.w; }

    inline glm::vec3 Kd() const { return glm::vec3(Mtl1); }
    inline float d()   const { return Mtl1.w; }

    inline glm::vec3 Ks() const { return glm::vec3(Mtl2); }
    inline float Ns()  const { return Mtl2.w; }

    inline glm::vec3 Ke() const { return glm::vec3(Mtl3); }
    inline float extra() const { return Mtl3.w; }
};

/**
 * @brief A universal material class that uses:
 *        - A MaterialLayout describing which params/textures to support
 *        - A bitmask to track which TextureType entries are actually set
 *        - A PackedMtlParams for standard MTL fields (Ambient, Diffuse, etc.)
 *        - An optional map of custom textures and custom parameters
 */
class Material {
public:
    explicit Material(const MaterialLayout& layout);
    ~Material() = default;

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    // Basic name & ID
    void         SetName(const std::string& name);
    const std::string& GetName() const;

    void         SetID(int id);
    int   GetID() const;

    // The layout describing which params/textures are valid
    const MaterialLayout& GetLayout() const;

    // Standard MTL param -> packed param assignment
    void AssignToPackedParams(MaterialParamType type, const UniformValue& value);

    // Setting / Getting textures
    void SetTexture(TextureType type, const std::shared_ptr<Graphics::ITexture>& texture);
    std::shared_ptr<Graphics::ITexture> GetTexture(TextureType type) const;

    // Binding / Unbinding
    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

    // (Optional) set custom param or custom texture
    void SetCustomParam(const std::string& paramName, UniformValue value);
    void SetCustomTexture(const std::string& uniformName, const std::shared_ptr<Graphics::ITexture>& texture);

private:
    // The ID and name
    int   m_ID = -1;
    std::string  m_Name;


    // Layout (which params, which textures)
    MaterialLayout m_Layout;

    // Standard MTL parameters
    PackedMtlParams m_PackedParams;

    // For other user-defined params
    std::unordered_map<std::string, UniformValue> m_CustomParams;

    // For standard textures: keyed by TextureType
    std::unordered_map<TextureType, std::shared_ptr<Graphics::ITexture>> m_Textures;

    // For custom textures, keyed by a user-chosen uniform name
    std::unordered_map<std::string, std::shared_ptr<Graphics::ITexture>> m_CustomTextures;

    // A bitmask that says which TextureType bits are used
    uint32_t m_TextureUsage = 0;
};
