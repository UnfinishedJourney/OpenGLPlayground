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

// A variant holding possible uniform parameter types.
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

namespace graphics {

    /**
     * @brief Standard packed material parameters.
     *
     * Layout:
     *   - mtl0_: (Ka.xyz, Ni)
     *   - mtl1_: (Kd.xyz, d)
     *   - mtl2_: (Ks.xyz, Ns)
     *   - mtl3_: (Ke.xyz, extra)
     */
    struct PackedMtlParams {
        glm::vec4 mtl0_{ 0.2f, 0.2f, 0.2f, 1.0f };
        glm::vec4 mtl1_{ 0.8f, 0.8f, 0.8f, 1.0f };
        glm::vec4 mtl2_{ 0.0f, 0.0f, 0.0f, 32.0f };
        glm::vec4 mtl3_{ 0.0f, 0.0f, 0.0f, 0.0f };

        inline glm::vec3 Ambient() const { return glm::vec3(mtl0_); }
        inline float Ni() const { return mtl0_.w; }
        inline glm::vec3 Diffuse() const { return glm::vec3(mtl1_); }
        inline float d() const { return mtl1_.w; }
        inline glm::vec3 Specular() const { return glm::vec3(mtl2_); }
        inline float Shininess() const { return mtl2_.w; }
        inline glm::vec3 Emissive() const { return glm::vec3(mtl3_); }
        inline float Extra() const { return mtl3_.w; }
    };

    /**
     * @brief Material class that stores both standard and custom parameters/textures.
     */
    class Material {
    public:
        explicit Material(const MaterialLayout& layout);
        ~Material() = default;

        // Non-copyable, but movable.
        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;
        Material(Material&&) noexcept = default;
        Material& operator=(Material&&) noexcept = default;

        // Name and ID accessors.
        void SetName(const std::string& name);
        const std::string& GetName() const;

        void SetID(std::size_t id);
        std::size_t GetID() const;

        const MaterialLayout& GetLayout() const;

        // Standard parameters.
        void AssignToPackedParams(MaterialParamType type, const UniformValue& value);

        // Standard textures.
        void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
        std::shared_ptr<ITexture> GetTexture(TextureType type) const;

        // Custom parameters and textures.
        void SetCustomParam(const std::string& param_name, UniformValue value);
        void SetCustomTexture(const std::string& uniform_name, const std::shared_ptr<ITexture>& texture);

        // Bind/unbind material for a given shader.
        void Bind(const std::shared_ptr<BaseShader>& shader) const;
        void Unbind() const;

    private:
        std::size_t id_{ 0 };
        std::string name_;
        MaterialLayout layout_;
        PackedMtlParams packed_params_;
        std::unordered_map<std::string, UniformValue> custom_params_;
        std::unordered_map<TextureType, std::shared_ptr<ITexture>> textures_;
        std::unordered_map<std::string, std::shared_ptr<ITexture>> custom_textures_;
        uint32_t texture_usage_{ 0 }; // Bitmask tracking which standard textures are set.
    };

} // namespace graphics