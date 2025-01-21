#include "Material.h"
#include "Utilities/Logger.h"
#include <glm/gtc/type_ptr.hpp>

// --------------------------------------------------------
//  MAPPING TEXTURE TYPE => GLSL Uniform Names
//  and binding slots
// --------------------------------------------------------
static std::string GetTextureUniformName(TextureType type)
{
    switch (type)
    {
    case TextureType::Albedo:         return "uTexAlbedo";
    case TextureType::Normal:         return "uTexNormal";
    case TextureType::MetalRoughness: return "uTexMetalRoughness";
    case TextureType::AO:             return "uTexAO";
    case TextureType::Emissive:       return "uTexEmissive";
    case TextureType::Ambient:        return "uTexAmbient";
    case TextureType::Height:         return "uTexHeight";
    case TextureType::BRDFLut:        return "uTexBRDFLUT";
    default:                          return "uTexUnknown";
    }
}

static int GetTextureBindingSlot(TextureType type)
{
    // You can assign them however you like:
    switch (type)
    {
    case TextureType::Albedo:         return 1;
    case TextureType::Normal:         return 2;
    case TextureType::MetalRoughness: return 3;
    case TextureType::AO:             return 4;
    case TextureType::Emissive:       return 5;
    case TextureType::Ambient:        return 6;
    case TextureType::Height:         return 7;
    case TextureType::BRDFLut:        return 8;
    default:                          return 10; // fallback
    }
}

// --------------------------------------------------------
//  Assign a param to the correct packed slot
// --------------------------------------------------------
void Material::AssignToPackedParams(MaterialParamType type, const UniformValue& value)
{
    // We interpret the variant as either float or vec3, etc.
    switch (type)
    {
    case MaterialParamType::Ambient: // Ka => Mtl0.xyz
    {
        // Usually user sets a vec3
        if (std::holds_alternative<glm::vec3>(value)) {
            glm::vec3 c = std::get<glm::vec3>(value);
            m_PackedParams.Mtl0.x = c.r;
            m_PackedParams.Mtl0.y = c.g;
            m_PackedParams.Mtl0.z = c.b;
        }
        else {
            Logger::GetLogger()->warn("Ambient must be vec3");
        }
    }
    break;

    case MaterialParamType::Diffuse: // Kd => Mtl1.xyz
    {
        if (std::holds_alternative<glm::vec3>(value)) {
            glm::vec3 c = std::get<glm::vec3>(value);
            m_PackedParams.Mtl1.x = c.r;
            m_PackedParams.Mtl1.y = c.g;
            m_PackedParams.Mtl1.z = c.b;
        }
        else {
            Logger::GetLogger()->warn("Diffuse must be vec3");
        }
    }
    break;

    case MaterialParamType::Specular: // Ks => Mtl2.xyz
    {
        if (std::holds_alternative<glm::vec3>(value)) {
            glm::vec3 c = std::get<glm::vec3>(value);
            m_PackedParams.Mtl2.x = c.r;
            m_PackedParams.Mtl2.y = c.g;
            m_PackedParams.Mtl2.z = c.b;
        }
        else {
            Logger::GetLogger()->warn("Specular must be vec3");
        }
    }
    break;

    case MaterialParamType::Shininess: // Ns => Mtl2.w
    {
        if (std::holds_alternative<float>(value)) {
            float shininess = std::get<float>(value);
            m_PackedParams.Mtl2.w = shininess;
        }
        else {
            Logger::GetLogger()->warn("Shininess must be float");
        }
    }
    break;

    default:
        Logger::GetLogger()->warn("Unhandled MaterialParamType in AssignToPackedParams");
        break;
    }
}


// --------------------------------------------------------
//  SetTexture => set bit in m_TextureUsage
// --------------------------------------------------------
void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("SetTexture: Null texture for type {}", (int)type);
        return;
    }

    // Check if layout includes that texture type
    if (m_Layout.textures.find(type) == m_Layout.textures.end()) {
        Logger::GetLogger()->warn("SetTexture: TextureType {} not in layout for material '{}'.",
            (int)type, m_Name);
        return;
    }

    m_Textures[type] = texture;

    // set the bit
    m_TextureUsage |= (1 << static_cast<std::size_t>(type));
}

std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const
{
    auto it = m_Textures.find(type);
    if (it != m_Textures.end()) {
        return it->second;
    }
    return nullptr;
}

// --------------------------------------------------------
//  Bind
// --------------------------------------------------------
void Material::Bind(const std::shared_ptr<BaseShader>& shader) const
{
    if (!shader) {
        Logger::GetLogger()->error("Bind: No shader provided for material '{}'.", m_Name);
        return;
    }

    // 1) Send the bitmask
    auto t = m_TextureUsage;

    shader->SetUniform("uMaterial_textureUsageFlags", (int)m_TextureUsage);

    // 2) Send the packed MTL parameters
    //    We'll call them Mtl0..Mtl3 in the shader
    shader->SetUniform("uMaterial.Mtl0", m_PackedParams.Mtl0);
    shader->SetUniform("uMaterial.Mtl1", m_PackedParams.Mtl1);
    shader->SetUniform("uMaterial.Mtl2", m_PackedParams.Mtl2);
    shader->SetUniform("uMaterial.Mtl3", m_PackedParams.Mtl3);

    // 3) Bind standard textures
    for (auto& [texType, texPtr] : m_Textures)
    {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Bind(slot);

        std::string uniformName = GetTextureUniformName(texType);
        shader->SetUniform(uniformName, slot);
    }

    // 4) Bind custom textures (beyond known texture types)
    //    We'll pick some starting slot after standard ones
    int customSlot = 10;
    for (auto& [customName, texPtr] : m_CustomTextures)
    {
        if (!texPtr) continue;
        texPtr->Bind(customSlot);
        shader->SetUniform(customName, customSlot);
        customSlot++;
    }

    // 5) Bind custom param uniforms (int, float, vec2, etc.)
    for (auto& [customParamName, customVal] : m_CustomParams)
    {
        // Use a visitor to set the uniform
        std::visit([&](auto&& arg) {
            shader->SetUniform(customParamName, arg);
            }, customVal);
    }
}

void Material::Unbind() const
{
    // For a simple scenario, just unbind the known texture slots
    for (auto& [texType, texPtr] : m_Textures) {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Unbind(slot);
    }
    // Also unbind the custom textures
    int customSlot = 10;
    for (auto& [customName, texPtr] : m_CustomTextures) {
        if (!texPtr) continue;
        texPtr->Unbind(customSlot);
        customSlot++;
    }
}