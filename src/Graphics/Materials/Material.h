#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <string_view>

#include "Graphics/Textures/TextureBase.h"
#include "Graphics/Shaders/BaseShader.h"
#include <glm/glm.hpp>

using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

class Material {
public:
    Material() = default;
    ~Material() = default;

    void AddTexture(const std::shared_ptr<TextureBase>& texture, GLuint textureUnit = 0);

    template <typename T>
    void AddParam(std::string_view name, const T& value);

    const std::unordered_map<std::string, UniformValue>& GetParams() const;
    const std::unordered_map<GLuint, std::shared_ptr<TextureBase>>& GetTextures() const;

    void Bind(const std::shared_ptr<BaseShader>& shader) const;
    void Unbind() const;

private:
    std::unordered_map<GLuint, std::shared_ptr<TextureBase>> m_Textures;
    std::unordered_map<std::string, UniformValue> m_Params;
};