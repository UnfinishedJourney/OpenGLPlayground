#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <string_view>

#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Shaders/BaseShader.h"
#include <glm/glm.hpp>

using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

class Material {
public:
    Material() = default;
    ~Material() = default;

    void AddTexture(std::weak_ptr<Texture2D> texture, GLuint textureUnit = 0);

    template <typename T>
    void AddParam(std::string_view name, T value);

    const std::unordered_map<std::string, UniformValue>& GetParams() const;
    const std::unordered_map<GLuint, std::weak_ptr<Texture2D>>& GetTextures() const;

    void Bind(std::shared_ptr<BaseShader> shader);
    void Unbind();

private:
    std::unordered_map<GLuint, std::weak_ptr<Texture2D>> m_Textures;
    std::unordered_map<std::string, UniformValue> m_Params;
};