#pragma once

#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Shaders/BaseShader.h"

#include <glm.hpp> 
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

class MaterialManager;

class Material {
    friend class MaterialManager;

public:
    Material() = default;
    ~Material() = default;

    void AddTexture(std::weak_ptr<Texture2D> texture, unsigned int textureUnit = 0);

    template<typename T>
    void AddParam(const std::string& name, T value);

    const std::unordered_map<std::string, UniformValue>& GetParams() const;
    const std::unordered_map<unsigned int, std::weak_ptr<Texture2D>>& GetTextures() const;

private:
    void Bind(std::shared_ptr<BaseShader> shader);
    void Unbind();

    std::unordered_map<unsigned int, std::weak_ptr<Texture2D>> m_Textures;
    std::unordered_map<std::string, UniformValue> m_Params;
};