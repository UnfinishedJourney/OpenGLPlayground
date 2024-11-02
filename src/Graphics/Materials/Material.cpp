#include "Graphics/Materials/Material.h"
#include "Utilities/Logger.h"
#include <glm/gtc/type_ptr.hpp>

void Material::AddTexture(const std::shared_ptr<Texture2D>& texture, GLuint textureUnit) {
    if (texture) {
        m_Textures[textureUnit] = texture;
        Logger::GetLogger()->info("Texture added to unit {}.", textureUnit);
    }
    else {
        Logger::GetLogger()->warn("Attempted to add a null texture to unit {}.", textureUnit);
    }
}

template <typename T>
void Material::AddParam(std::string_view name, const T& value) {
    m_Params.emplace(std::string(name), value);
    Logger::GetLogger()->info("Parameter '{}' added.", name);
}

// Explicit template instantiations
template void Material::AddParam<int>(std::string_view name, const int& value);
template void Material::AddParam<float>(std::string_view name, const float& value);
template void Material::AddParam<glm::vec2>(std::string_view name, const glm::vec2& value);
template void Material::AddParam<glm::vec3>(std::string_view name, const glm::vec3& value);
template void Material::AddParam<glm::vec4>(std::string_view name, const glm::vec4& value);
template void Material::AddParam<glm::mat3>(std::string_view name, const glm::mat3& value);
template void Material::AddParam<glm::mat4>(std::string_view name, const glm::mat4& value);

const std::unordered_map<std::string, UniformValue>& Material::GetParams() const {
    return m_Params;
}

const std::unordered_map<GLuint, std::shared_ptr<Texture2D>>& Material::GetTextures() const {
    return m_Textures;
}

void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
    if (!shader) {
        Logger::GetLogger()->error("Attempted to bind material to a null shader.");
        return;
    }

    Logger::GetLogger()->debug("Binding material to shader.");

    // Bind textures
    for (const auto& [unit, texture] : m_Textures) {
        if (texture) {
            texture->Bind(unit);
            Logger::GetLogger()->debug("Texture bound to unit {}.", unit);
        }
        else {
            Logger::GetLogger()->warn("Texture in unit {} is null. Skipping bind.", unit);
        }
    }

    // Set uniform parameters
    for (const auto& [name, value] : m_Params) {
        std::visit([&](const auto& arg) {
            shader->SetUniform(name, arg);
            Logger::GetLogger()->debug("Set uniform '{}' with value.", name);
            }, value);
    }
}

void Material::Unbind() const {
    Logger::GetLogger()->debug("Unbinding material textures.");

    for (const auto& [unit, texture] : m_Textures) {
        if (texture) {
            texture->Unbind(unit);
            Logger::GetLogger()->debug("Texture unbound from unit {}.", unit);
        }
    }
}