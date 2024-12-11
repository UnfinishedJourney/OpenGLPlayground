#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <string_view>
#include <glm/glm.hpp>
#include "Graphics/Textures/ITexture.h"
#include "Graphics/Shaders/BaseShader.h"

using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

class Material {
public:
    Material() = default;
    ~Material() = default;

    void AddTexture(const std::shared_ptr<ITexture>& texture, uint32_t unit = 0);
    template<typename T> void AddParam(const std::string& name, const T& value);

    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

private:
    std::unordered_map<uint32_t, std::shared_ptr<ITexture>> m_Textures;
    std::unordered_map<std::string, UniformValue> m_Params;
};