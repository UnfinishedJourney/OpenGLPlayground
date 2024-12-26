#include "Material.h"
#include "Utilities/Logger.h"

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
    case TextureType::Albedo:         return 0;
    case TextureType::Normal:         return 1;
    case TextureType::MetalRoughness: return 2;
    case TextureType::AO:             return 3;
    case TextureType::Emissive:       return 4;
    case TextureType::BRDFLut:        return 5; 
    default:                          return 10; // fallback for anything else
    }
}

void Material::SetParam(MaterialParamType type, const UniformValue& value)
{
    m_Params[type] = value;
    m_Layout.params.insert(type);
}

void Material::SetCustomParam(const std::string& name, const UniformValue& value)
{
    m_CustomParams[name] = value;
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
        Logger::GetLogger()->warn("Attempted to set null texture for type {}.", (int)type);
        return;
    }
    m_Textures[type] = texture;
    m_Layout.textures.insert(type);
}

void Material::SetCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("Attempted to set null custom texture '{}'.", name);
        return;
    }
    m_CustomTextures[name] = texture;
}

std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const
{
    auto it = m_Textures.find(type);
    return (it != m_Textures.end()) ? it->second : nullptr;
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
    for (auto& [texType, texPtr] : m_Textures)
    {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Bind(slot);

        std::string uniformName = GetTextureUniformName(texType);
        shader->SetUniform(uniformName, slot);
    }

    // 2. Bind custom textures
    //    Let’s pick a start slot offset beyond the standard ones, e.g. 10
    int customSlot = 10;
    for (auto& [customName, texPtr] : m_CustomTextures)
    {
        if (!texPtr) continue;
        texPtr->Bind(customSlot);
        shader->SetUniform(customName, customSlot);
        ++customSlot;
    }

    // 3. Bind standard parameters
    for (auto& [paramType, value] : m_Params)
    {
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
            // ...
            break;
        }
    }

    // 4. Bind custom params
    for (auto& [customName, val] : m_CustomParams)
    {
        std::visit([&](auto&& arg) {
            shader->SetUniform(customName, arg);
            }, val);
    }
}

void Material::Unbind() const
{
    // 1. Unbind standard textures
    for (auto& [texType, texPtr] : m_Textures)
    {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Unbind(slot);
    }

    // 2. Unbind custom textures
    int customSlot = 10;
    for (auto& [customName, texPtr] : m_CustomTextures)
    {
        if (!texPtr) continue;
        texPtr->Unbind(customSlot);
        ++customSlot;
    }
}