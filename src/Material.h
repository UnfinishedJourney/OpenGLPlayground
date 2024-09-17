#pragma once

#include "Shader.h"
#include "Texture.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>

class Material {
public:
    void SetShader(std::shared_ptr<Shader> shader) {
        m_Shader = shader;
    }

    void AddTexture(std::shared_ptr<Texture> texture) {
        m_Textures.push_back(texture);
    }

    void AddParam(std::string name, float value) {
        m_Params[name] = value;
    }

    void Bind() {
        if (m_IsBound)
            return;

        m_Shader->Bind();

        for (auto& [key, value] : m_Params)
        {
            std::stringstream name;
            name << "Material" << "." << key;
            m_Shader->SetUniform(name.str(), value);
        }

        for (size_t i = 0; i < m_Textures.size(); ++i) {
            m_Textures[i]->Bind(i);
        }
    }

    std::shared_ptr<Shader> GetShader() const {
        return m_Shader;
    }

private:
    std::shared_ptr<Shader> m_Shader;
    std::vector<std::shared_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, float> m_Params;
    mutable bool m_IsBound = false;
};