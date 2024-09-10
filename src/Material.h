#pragma once

#include "Shader.h"
#include "Texture.h"

#include <memory>
#include <string>
#include <unordered_map>

class Material {
public:
    void SetShader(std::shared_ptr<Shader> shader) {
        m_Shader = shader;
    }

    void AddTexture(std::shared_ptr<Texture> texture) {
        m_Textures.push_back(texture);
    }

    void Bind() {
        m_Shader->Bind();

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
};