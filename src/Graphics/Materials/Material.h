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
     *   - mtl0_: (Ambient.xyz, RefractionIndex)
     *   - mtl1_: (Diffuse.xyz, Opacity)
     *   - mtl2_: (Specular.xyz, Shininess)
     *   - mtl3_: (Emissive.xyz, Illumination)
     */
    struct PackedMtlParams {
        glm::vec4 mtl0_{ 0.2f, 0.2f, 0.2f, 1.0f };
        glm::vec4 mtl1_{ 0.8f, 0.8f, 0.8f, 1.0f };
        glm::vec4 mtl2_{ 0.0f, 0.0f, 0.0f, 32.0f };
        glm::vec4 mtl3_{ 0.0f, 0.0f, 0.0f, 0.0f };

        inline glm::vec3 Ambient() const { return glm::vec3(mtl0_); }
        inline float RefractionIndex() const { return mtl0_.w; }
        inline glm::vec3 Diffuse() const { return glm::vec3(mtl1_); }
        inline float Opacity() const { return mtl1_.w; }
        inline glm::vec3 Specular() const { return glm::vec3(mtl2_); }
        inline float Shininess() const { return mtl2_.w; }
        inline glm::vec3 Emissive() const { return glm::vec3(mtl3_); }
        inline float Illumination() const { return mtl3_.w; }
    };

    /**
     * @brief Material class storing both standard and custom parameters/textures.
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

        void SetName(const std::string& name);
        const std::string& GetName() const;

        void SetID(std::size_t id);
        std::size_t GetID() const;

        const MaterialLayout& GetLayout() const;

        void AssignToPackedParams(MaterialParamType type, const UniformValue& value);
        void SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture);
        std::shared_ptr<ITexture> GetTexture(TextureType type) const;
        void SetCustomParam(const std::string& paramName, UniformValue value);
        void SetCustomTexture(const std::string& uniformName, const std::shared_ptr<ITexture>& texture);
        void Bind(const std::shared_ptr<BaseShader>& shader) const;
        void Unbind() const;

    private:
        std::size_t id_ = 0;
        std::string name_;
        MaterialLayout layout_;
        PackedMtlParams packedParams_;
        std::unordered_map<std::string, UniformValue> customParams_;
        std::unordered_map<TextureType, std::shared_ptr<ITexture>> textures_;
        std::unordered_map<std::string, std::shared_ptr<ITexture>> customTextures_;
        uint32_t textureUsage_ = 0;
    };

} // namespace graphics