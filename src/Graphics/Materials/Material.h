#pragma once

#include <variant>
#include <unordered_map>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include "MaterialParamType.h"
#include "MaterialLayout.h"
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

// The typical "material ID" type
using MaterialID = int;

// UniformValue holds any parameter type
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

/**
 * @brief A small struct to pack Ka, Kd, Ks, Ke, Ns, d, Ni, etc.
 *        into four vec4.
 */
struct PackedMtlParams
{
    glm::vec4 Mtl0 = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f); // default: Ka.xyz, Ni in .w
    glm::vec4 Mtl1 = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); // default: Kd.xyz, d in .w
    glm::vec4 Mtl2 = glm::vec4(0.0f, 0.0f, 0.0f, 32.0f); // Ks.xyz, Ns in .w
    glm::vec4 Mtl3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);  // Ke.xyz, spare in .w

    // Helpers to interpret the data if you want in C++:
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
 * @brief A universal Material class that uses a bitmask for textures
 *        and a PackedMtlParams for the standard MTL parameters.
 */
class Material {
public:
    // Constructors / destructor
    Material(const MaterialLayout& layout)
        : m_Layout(layout)
    {}

    ~Material() = default;

    // Basic name & ID
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }

    void SetID(MaterialID id) { m_ID = id; }
    MaterialID GetID() const { return m_ID; }

    const MaterialLayout& GetLayout() const { return m_Layout; }

    // ======================================================
    //  2) Textures / bitmask
    // ======================================================
    void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
    std::shared_ptr<ITexture> GetTexture(TextureType type) const;


    // ======================================================
    //  3) Bind / Unbind
    // ======================================================
    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

    void AssignToPackedParams(MaterialParamType type, const UniformValue& value);

private:
    // Helper to interpret MTL param => correct slot in m_PackedParams


private:
    MaterialID     m_ID = -1;
    std::string    m_Name;
    MaterialLayout m_Layout;

    // For standard MTL params in packed form
    PackedMtlParams m_PackedParams;

    // For "other" user-defined param values
    std::unordered_map<std::string, UniformValue> m_CustomParams;

    // For standard textures (by type)
    std::unordered_map<TextureType, std::shared_ptr<ITexture>> m_Textures;

    // For custom-named textures
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_CustomTextures;

    // This bitmask says which TextureType bits are used
    uint32_t m_TextureUsage = 0;

};