#include "Material.h"

// Helper: map each TextureType to a known GLSL uniform name & binding slot
static std::string GetTextureUniformName(TextureType type)
{
    switch (type)
    {
    case TextureType::Albedo:         return "texAlbedo";
    case TextureType::Normal:         return "texNormal";
    case TextureType::MetalRoughness: return "texMetalRoughness";
    case TextureType::AO:             return "texAO";
    case TextureType::Emissive:       return "texEmissive";
    case TextureType::BRDFLut:        return "texBRDF_LUT";
    default:                          return "texUnknown";
    }
}

static int GetTextureBindingSlot(TextureType type)
{
    switch (type)
    {
    case TextureType::Albedo:           return 1;
    case TextureType::Normal:           return 2;
    case TextureType::MetalRoughness:   return 3;
    case TextureType::AO:               return 4;
    case TextureType::Emissive:         return 5;
    case TextureType::Ambient:          return 6;
    case TextureType::Height:           return 7;
    case TextureType::BRDFLut:          return 8;
    default:                            return 10; // fallback
    }
}

void Material::SetParam(MaterialParamType type, const UniformValue& value)
{
    if (m_Layout.params.find(type) != m_Layout.params.end()) {
        m_Params[type] = value;
    }
    
    else {
        Logger::GetLogger()->warn("Attempted to set wrong parameter of type {}.", static_cast<int>(type));
    }
}

bool Material::GetParam(MaterialParamType type, UniformValue& outValue) const
{
    auto it = m_Params.find(type);
    if (it != m_Params.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

void Material::SetCustomParam(const std::string& name, const UniformValue& value)
{
    m_CustomParams[name] = value;
}

bool Material::GetCustomParam(const std::string& name, UniformValue& outValue) const
{
    auto it = m_CustomParams.find(name);
    if (it != m_CustomParams.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null texture for type {}.", static_cast<int>(type));
        return;
    }

    if (m_Layout.textures.find(type) != m_Layout.textures.end()) {
        m_Textures[type] = texture;
    }
    else {
        Logger::GetLogger()->warn("Attempted to set wrong wrong of type {}.", static_cast<int>(type));
    }
}

std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const
{
    auto it = m_Textures.find(type);
    return (it != m_Textures.end()) ? it->second : nullptr;
}

void Material::SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null custom texture '{}'.", name);
        return;
    }
    m_CustomTextures[name] = texture;
}

std::shared_ptr<ITexture> Material::GetCustomTexture(const std::string& name) const
{
    auto it = m_CustomTextures.find(name);
    return (it != m_CustomTextures.end()) ? it->second : nullptr;
}

void Material::Bind(const std::shared_ptr<BaseShader>& shader) const
{
    if (!shader) {
        Logger::GetLogger()->error("No shader provided to material bind.");
        return;
    }

    // 1. Bind standard textures
    for (const auto& [texType, texPtr] : m_Textures) {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Bind(slot);

        std::string uniformName = GetTextureUniformName(texType);
        shader->SetUniform(uniformName, slot);
    }

    // 2. Bind custom textures (slots start after the standard ones, e.g. 10)
    int customSlot = 10;
    for (const auto& [customName, texPtr] : m_CustomTextures) {
        if (!texPtr) continue;
        texPtr->Bind(customSlot);
        shader->SetUniform(customName, customSlot);
        ++customSlot;
    }

    // 3. Bind standard parameters
    for (const auto& [paramType, value] : m_Params) {
        switch (paramType)
        {
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
        default:
            Logger::GetLogger()->warn("Unhandled MaterialParamType {}.", static_cast<int>(paramType));
            break;
        }
    }

    // 4. Bind custom parameters
    for (const auto& [customName, val] : m_CustomParams) {
        std::visit([&](auto&& arg) {
            shader->SetUniform(customName, arg);
            }, val);
    }
}

void Material::Unbind() const
{
    // 1. Unbind standard textures
    for (const auto& [texType, texPtr] : m_Textures) {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Unbind(slot);
    }

    // 2. Unbind custom textures
    int customSlot = 10;
    for (const auto& [customName, texPtr] : m_CustomTextures) {
        if (!texPtr) continue;
        texPtr->Unbind(customSlot);
        ++customSlot;
    }
}