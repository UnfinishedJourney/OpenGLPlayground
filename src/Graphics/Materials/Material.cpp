#include "Material.h"
#include "Utilities/Logger.h"

void Material::SetParam(MaterialParamType type, const UniformValue& value) {
    m_Params[type] = value;
    m_Layout.params.insert(type);
}

void Material::SetCustomParam(const std::string& name, const UniformValue& value) {
    m_CustomParams[name] = value;
    // Custom parameters are tracked implicitly by their string names
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
    texture->Bind(unit);
}

void Material::SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture, uint32_t unit) {
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null custom texture.");
        return;
    }
    m_CustomTextures[name] = texture;
    // Custom textures are tracked by their string names
    texture->Bind(unit);
}

std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const {
    auto it = m_Textures.find(type);
    if (it != m_Textures.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<ITexture> Material::GetCustomTexture(const std::string& name) const {
    auto it = m_CustomTextures.find(name);
    if (it != m_CustomTextures.end()) {
        return it->second;
    }
    return nullptr;
}

void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
    if (!shader) {
        Logger::GetLogger()->error("No shader provided to material bind.");
        return;
    }

    // Bind standard textures
    for (const auto& [type, tex] : m_Textures) {
        uint32_t unit = static_cast<uint32_t>(type); // Ensure mapping is correct
        tex->Bind(unit);
        // Assume uniform names follow a convention like "textureAlbedo", "textureNormal", etc.
        std::string uniformName;
        switch (type) {
        case TextureType::Albedo: uniformName = "textureAlbedo"; break;
        case TextureType::Normal: uniformName = "textureNormal"; break;
        case TextureType::MetalRoughness: uniformName = "textureMetalRoughness"; break;
        case TextureType::AO: uniformName = "textureAO"; break;
        case TextureType::Emissive: uniformName = "textureEmissive"; break;
        default: continue; // Skip unknown types
        }
        shader->SetUniform(uniformName, static_cast<int>(unit));
    }

    // Bind custom textures
    uint32_t customUnit = static_cast<uint32_t>(m_Textures.size()); // Start after standard units
    for (const auto& [name, tex] : m_CustomTextures) {
        tex->Bind(customUnit);
        shader->SetUniform(name, static_cast<int>(customUnit));
        customUnit++;
    }

    // Set standard parameters
    for (const auto& [type, val] : m_Params) {
        switch (type) {
        case MaterialParamType::Ambient:
            shader->SetUniform("material.Ka", std::get<glm::vec3>(val));
            break;
        case MaterialParamType::Diffuse:
            shader->SetUniform("material.Kd", std::get<glm::vec3>(val));
            break;
        case MaterialParamType::Specular:
            shader->SetUniform("material.Ks", std::get<glm::vec3>(val));
            break;
        case MaterialParamType::Shininess:
            shader->SetUniform("material.shininess", std::get<float>(val));
            break;
        case MaterialParamType::Roughness:
            shader->SetUniform("material.roughness", std::get<float>(val));
            break;
        case MaterialParamType::Metallic:
            shader->SetUniform("material.metallic", std::get<float>(val));
            break;
        case MaterialParamType::Emissive:
            shader->SetUniform("material.emissive", std::get<glm::vec3>(val));
            break;
            // No 'Custom' type
        }
    }

    // Set custom parameters
    for (const auto& [name, val] : m_CustomParams) {
        std::visit([&](auto&& arg) {
            shader->SetUniform(name, arg);
            }, val);
    }
}

void Material::Unbind() const {
    // Unbind standard textures
    for (const auto& [type, tex] : m_Textures) {
        uint32_t unit = static_cast<uint32_t>(type);
        tex->Unbind(unit);
    }

    // Unbind custom textures
    uint32_t customUnit = static_cast<uint32_t>(m_Textures.size());
    for (const auto& [name, tex] : m_CustomTextures) {
        tex->Unbind(customUnit);
        customUnit++;
    }
}