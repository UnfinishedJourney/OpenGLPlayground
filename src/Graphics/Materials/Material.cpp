#include "Graphics/Materials/Material.h"
#include "Utilities/Logger.h"

void Material::AddTexture(std::weak_ptr<Texture2D> texture, GLuint textureUnit) {
    if (auto sharedTexture = texture.lock()) {
        m_Textures[textureUnit] = std::move(texture);
        Logger::GetLogger()->info("Texture added to unit {}.", textureUnit);
    }
    else {
        Logger::GetLogger()->warn("Attempted to add an expired texture to unit {}.", textureUnit);
    }
}

template <typename T>
void Material::AddParam(std::string_view name, T value) {
    m_Params[std::string(name)] = value;
    Logger::GetLogger()->info("Parameter '{}' added with value.", name);
}

// Explicit template instantiations
template void Material::AddParam<int>(std::string_view name, int value);
template void Material::AddParam<float>(std::string_view name, float value);
template void Material::AddParam<glm::vec2>(std::string_view name, glm::vec2 value);
template void Material::AddParam<glm::vec3>(std::string_view name, glm::vec3 value);
template void Material::AddParam<glm::vec4>(std::string_view name, glm::vec4 value);
template void Material::AddParam<glm::mat4>(std::string_view name, glm::mat4 value);

const std::unordered_map<std::string, UniformValue>& Material::GetParams() const {
    return m_Params;
}

const std::unordered_map<GLuint, std::weak_ptr<Texture2D>>& Material::GetTextures() const {
    return m_Textures;
}

void Material::Bind(std::shared_ptr<BaseShader> shader) {
    if (!shader) {
        Logger::GetLogger()->error("Attempted to bind material to a null shader.");
        return;
    }

    Logger::GetLogger()->info("Binding material to shader.");

    for (const auto& [name, value] : m_Params) {
        std::string uniformName = "Material." + name;
        std::visit([&](auto&& arg) {
            shader->SetUniform(uniformName, arg);
            Logger::GetLogger()->debug("Set uniform '{}' with value.", uniformName);
            }, value);
    }

    for (const auto& [unit, weakTexture] : m_Textures) {
        if (auto texture = weakTexture.lock()) {
            texture->Bind(unit);
            Logger::GetLogger()->info("Texture bound to unit {}.", unit);
        }
        else {
            Logger::GetLogger()->warn("Texture in unit {} has expired. Skipping bind.", unit);
        }
    }
}

void Material::Unbind() {
    Logger::GetLogger()->info("Unbinding material textures.");

    for (const auto& [unit, weakTexture] : m_Textures) {
        if (auto texture = weakTexture.lock()) {
            texture->Unbind(unit);
            Logger::GetLogger()->info("Texture unbound from unit {}.", unit);
        }
    }
}