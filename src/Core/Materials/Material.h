#pragma once

#include "Shader.h"
#include "Texture2D.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>
#include <functional>

class Material {
public:
    void SetShader(std::shared_ptr<Shader> shader) {
        m_Shader = shader;
    }

    void AddTexture(std::shared_ptr<Texture2D> texture, unsigned int textureUnit = 0) {
        m_Textures[textureUnit] = texture;
    }

    template<typename T>
    void AddParam(const std::string& name, T value) {
        m_Params[name] = [value](Shader& shader, const std::string& name) {
            std::string new_name = "Material." + name;
            shader.SetUniform(new_name, static_cast<T>(value));
        };
    }

    void Bind() {
        if (m_IsBound)
            return;

        m_Shader->Bind();

        for (const auto& [name, applyUniform] : m_Params) {
            applyUniform(*m_Shader, name);
        }

        for (auto& [key, value] : m_Textures) {
            value->Bind(key);
        }
    }

    std::shared_ptr<Shader> GetShader() const {
        return m_Shader;
    }

private:
    std::shared_ptr<Shader> m_Shader;
    std::unordered_map<unsigned int, std::shared_ptr<Texture2D>> m_Textures;
    std::unordered_map<std::string, std::function<void(Shader&, const std::string&)>> m_Params;
    mutable bool m_IsBound = false;
};