#include "Material.h"
#include "Utilities/Logger.h"

void Material::SetParam(MaterialParamType type, const UniformValue& value) {
    m_Params[type] = value;
    m_Layout.params.insert(type);
}

void Material::SetCustomParam(const std::string& name, const UniformValue& value) {
    m_CustomParams[name] = value;
}

bool Material::GetParam(MaterialParamType type, UniformValue& outValue) const {
    auto it = m_Params.find(type);
    if (it != m_Params.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

bool Material::GetCustomParam(const std::string& name, UniformValue& outValue) const {
    auto it = m_CustomParams.find(name);
    if (it != m_CustomParams.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture, uint32_t unit) {
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null texture.");
        return;
    }
    m_Textures[type] = texture;
    m_Layout.textures.insert(type);
    // Typically, binding is done in Bind(), so we don't bind it here permanently.
}

void Material::SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture, uint32_t unit) {
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null custom texture.");
        return;
    }
    m_CustomTextures[name] = texture;
    // We'll bind it in Bind().
}

std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const {
    auto it = m_Textures.find(type);
    return (it != m_Textures.end()) ? it->second : nullptr;
}

std::shared_ptr<ITexture> Material::GetCustomTexture(const std::string& name) const {
    auto it = m_CustomTextures.find(name);
    return (it != m_CustomTextures.end()) ? it->second : nullptr;
}

void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
    if (!shader) {
        Logger::GetLogger()->error("No shader provided to material bind.");
        return;
    }
    // Bind standard textures with appropriate uniform names
    for (auto& [texType, texPtr] : m_Textures) {
        uint32_t unit = static_cast<uint32_t>(texType); // simplistic
        texPtr->Bind(unit);

        // You could map TextureType -> uniform string here
        std::string uniformName;
        switch (texType) {
        case TextureType::Albedo:         uniformName = "textureAlbedo";        break;
        case TextureType::Normal:         uniformName = "textureNormal";        break;
        case TextureType::MetalRoughness: uniformName = "textureMetalRoughness"; break;
        case TextureType::AO:             uniformName = "textureAO";            break;
        case TextureType::Emissive:       uniformName = "textureEmissive";      break;
        default:                          uniformName = "textureUnknown";       break;
        }
        //shader->SetUniform(uniformName, static_cast<int>(unit));
    }

    // Bind custom textures
    uint32_t customUnit = static_cast<uint32_t>(m_Textures.size());
    for (auto& [customName, texPtr] : m_CustomTextures) {
        texPtr->Bind(customUnit);
        shader->SetUniform(customName, static_cast<int>(customUnit));
        ++customUnit;
    }

    // Bind standard parameters
    for (auto& [paramType, value] : m_Params) {
        switch (paramType) {
        case MaterialParamType::Ambient:
            shader->SetUniform("material.Ka", std::get<glm::vec3>(value));
            break;
        case MaterialParamType::Diffuse:
            shader->SetUniform("material.Kd", std::get<glm::vec3>(value));
            break;
        case MaterialParamType::Specular:
            shader->SetUniform("material.Ks", std::get<glm::vec3>(value));
            break;
        case MaterialParamType::Shininess:
            shader->SetUniform("material.shininess", std::get<float>(value));
            break;
        }
    }

    // Bind custom parameters
    for (auto& [customName, val] : m_CustomParams) {
        std::visit([&](auto&& arg) {
            shader->SetUniform(customName, arg);
            }, val);
    }
}

void Material::Unbind() const {
    // Unbind standard textures
    for (auto& [texType, texPtr] : m_Textures) {
        uint32_t unit = static_cast<uint32_t>(texType);
        texPtr->Unbind(unit);
    }
    // Unbind custom textures
    uint32_t customUnit = static_cast<uint32_t>(m_Textures.size());
    for (auto& [customName, texPtr] : m_CustomTextures) {
        texPtr->Unbind(customUnit);
        ++customUnit;
    }
}