#include "Material.h"
#include "Utilities/Logger.h"

void Material::AddTexture(const std::shared_ptr<ITexture>& texture, uint32_t unit) {
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to add null texture.");
        return;
    }
    m_Textures[unit] = texture;
}

template<typename T>
void Material::AddParam(const std::string& name, const T& value) {
    m_Params[name] = value;
}
template void Material::AddParam<int>(const std::string&, const int&);
template void Material::AddParam<float>(const std::string&, const float&);
template void Material::AddParam<glm::vec2>(const std::string&, const glm::vec2&);
template void Material::AddParam<glm::vec3>(const std::string&, const glm::vec3&);
template void Material::AddParam<glm::vec4>(const std::string&, const glm::vec4&);
template void Material::AddParam<glm::mat3>(const std::string&, const glm::mat3&);
template void Material::AddParam<glm::mat4>(const std::string&, const glm::mat4&);

void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
    if (!shader) {
        Logger::GetLogger()->error("No shader provided to material bind.");
        return;
    }
    // Bind textures
    for (auto& [unit, tex] : m_Textures) {
        tex->Bind(unit);
    }
    // set uniforms
    for (auto& [name, val] : m_Params) {
        std::visit([&](auto&& arg) {
            shader->SetUniform(name, arg);
            }, val);
    }
}

void Material::Unbind() const {
    for (auto& [unit, tex] : m_Textures) {
        tex->Unbind(unit);
    }
}