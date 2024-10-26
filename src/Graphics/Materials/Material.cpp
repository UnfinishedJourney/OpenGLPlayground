#include "Graphics/Materials/Material.h"
#include "Utilities/Logger.h"
#include "Graphics/Shaders/BaseShader.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

void Material::AddTexture(std::weak_ptr<Texture2D> texture, unsigned int textureUnit)
{
    if (auto sharedTexture = texture.lock()) { 
        m_Textures[textureUnit] = texture;
        Logger::GetLogger()->info("Texture added to unit {}.", textureUnit);
    }
    else {
        Logger::GetLogger()->warn("Attempted to add an expired texture to unit {}.", textureUnit);
    }
}

template<typename T>
void Material::AddParam(const std::string& name, T value)
{
    m_Params[name] = value;
    Logger::GetLogger()->info("Parameter '{}' added with value.", name);
}

template void Material::AddParam<int>(const std::string& name, int value);
template void Material::AddParam<float>(const std::string& name, float value);
template void Material::AddParam<glm::vec2>(const std::string& name, glm::vec2 value);
template void Material::AddParam<glm::vec3>(const std::string& name, glm::vec3 value);
template void Material::AddParam<glm::vec4>(const std::string& name, glm::vec4 value);
template void Material::AddParam<glm::mat4>(const std::string& name, glm::mat4 value);

const std::unordered_map<std::string, UniformValue>& Material::GetParams() const
{
    return m_Params;
}

const std::unordered_map<unsigned int, std::weak_ptr<Texture2D>>& Material::GetTextures() const
{
    return m_Textures;
}

void Material::Bind(std::shared_ptr<BaseShader> shader)
{
    if (!shader) {
        Logger::GetLogger()->error("Attempted to bind material to a null shader.");
        return;
    }

    Logger::GetLogger()->info("Binding material to shader '{}'.", "unknown");

    for (const auto& [name, value] : m_Params)
    {
        std::string uniformName = "Material." + name;
        std::visit([&](auto&& arg) {
            shader->SetUniform(uniformName, arg);
            Logger::GetLogger()->debug("Set uniform '{}' with value.", uniformName);
            }, value);
    }

    for (const auto& [unit, weakTexture] : m_Textures)
    {
        if (auto texture = weakTexture.lock()) { 
            texture->Bind(unit);
            Logger::GetLogger()->info("Texture bound to unit {}.", unit);
        }
        else {
            Logger::GetLogger()->warn("Texture in unit {} has expired. Skipping bind.", unit);
        }
    }
}

void Material::Unbind()
{
    Logger::GetLogger()->info("Unbinding material textures.");

    for (const auto& [unit, weakTexture] : m_Textures)
    {
        if (auto texture = weakTexture.lock()) { 
            texture->Unbind(unit); 
            Logger::GetLogger()->info("Texture unbound from unit {}.", unit);
        }
    }

}