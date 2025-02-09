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

// A variant holding possible uniform parameter types.
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

namespace Graphics {

    /**
     * @brief Packed material parameters for standard MTL fields.
     *
     * Layout:
     *   Mtl0 = (Ka.xyz, Ni)
     *   Mtl1 = (Kd.xyz, d)
     *   Mtl2 = (Ks.xyz, Ns)
     *   Mtl3 = (Ke.xyz, extra)
     */
    struct PackedMtlParams {
        glm::vec4 Mtl0 = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 Mtl1 = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        glm::vec4 Mtl2 = glm::vec4(0.0f, 0.0f, 0.0f, 32.0f);
        glm::vec4 Mtl3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        inline glm::vec3 Ambient() const { return glm::vec3(Mtl0); }
        inline float Ni() const { return Mtl0.w; }
        inline glm::vec3 Diffuse() const { return glm::vec3(Mtl1); }
        inline float d() const { return Mtl1.w; }
        inline glm::vec3 Specular() const { return glm::vec3(Mtl2); }
        inline float Shininess() const { return Mtl2.w; }
        inline glm::vec3 Emissive() const { return glm::vec3(Mtl3); }
        inline float Extra() const { return Mtl3.w; }
    };

    /**
     * @brief A universal material class.
     *
     * Stores:
     *   - A MaterialLayout (which parameters and textures are valid)
     *   - Packed standard parameters (MTL)
     *   - Custom parameters (by name) and custom textures (by uniform name)
     *   - A bitmask tracking which standard texture types are used.
     */
    class Material {
    public:
        explicit Material(const MaterialLayout& layout);
        ~Material() = default;

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;
        Material(Material&&) = default;
        Material& operator=(Material&&) = default;

        // Basic name and ID
        void SetName(const std::string& name);
        const std::string& GetName() const;

        void SetID(int id);
        int GetID() const;

        // Retrieve the material layout.
        const MaterialLayout& GetLayout() const;

        // Assign a standard parameter to the packed parameters.
        void AssignToPackedParams(MaterialParamType type, const UniformValue& value);

        // Standard textures: set/get by TextureType.
        void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
        std::shared_ptr<ITexture> GetTexture(TextureType type) const;

        // Custom parameters and textures.
        void SetCustomParam(const std::string& paramName, UniformValue value);
        void SetCustomTexture(const std::string& uniformName, const std::shared_ptr<ITexture>& texture);

        // Bind/Unbind the material for a given shader.
        void Bind(const std::shared_ptr<BaseShader>& shader) const;
        void Unbind() const;

    private:
        int m_ID = -1;
        std::string m_Name;
        MaterialLayout m_Layout;
        PackedMtlParams m_PackedParams;
        std::unordered_map<std::string, UniformValue> m_CustomParams;
        std::unordered_map<TextureType, std::shared_ptr<ITexture>> m_Textures;
        std::unordered_map<std::string, std::shared_ptr<ITexture>> m_CustomTextures;
        uint32_t m_TextureUsage = 0; // Bitmask (each bit represents a TextureType)
    };

} // namespace Graphics