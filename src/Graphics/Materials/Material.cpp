#include "Material.h"
#include <glm/gtc/type_ptr.hpp>
#include "Resources/TextureManager.h"
#include "Utilities/Logger.h"

// --------------------------------------------------------
//  Helper: map a TextureType to the uniform name in GLSL
// --------------------------------------------------------
static std::string GetTextureUniformName(TextureType type)
{
    switch (type) {
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

// --------------------------------------------------------
//  Helper: map a TextureType to a texture binding slot
// --------------------------------------------------------
static int GetTextureBindingSlot(TextureType type)
{
    switch (type) {
    case TextureType::Albedo:           return 0;
    case TextureType::Normal:           return 1;
    case TextureType::MetalRoughness:   return 2;
    case TextureType::AO:               return 3;
    case TextureType::Emissive:         return 4;
    case TextureType::Ambient:          return 5;
    case TextureType::Height:           return 6;
    case TextureType::BRDFLut:          return 7;
    case TextureType::SkyBox:           return 8;
    case TextureType::SkyboxIrradiance: return 9;
    case TextureType::ShadowMap:        return 10;
    default:                            return 11; // fallback
    }
}



// --------------------------------------------------------
//  Material constructor
// --------------------------------------------------------
Material::Material(const MaterialLayout& layout)
    : m_Layout(layout)
{
}

// --------------------------------------------------------
//  Basic name & ID
// --------------------------------------------------------
void Material::SetName(const std::string& name)
{
    m_Name = name;
}

const std::string& Material::GetName() const
{
    return m_Name;
}

void Material::SetID(int id)
{
    m_ID = id;
}

int Material::GetID() const
{
    return m_ID;
}

const MaterialLayout& Material::GetLayout() const
{
    return m_Layout;
}


// --------------------------------------------------------
//  AssignToPackedParams => fill PackedMtlParams
// --------------------------------------------------------
void Material::AssignToPackedParams(MaterialParamType type, const UniformValue& value)
{
    // We interpret the variant and put the data in the right part of m_PackedParams
    switch (type) {
    case MaterialParamType::Ambient: { // Ka => Mtl0.xyz
        if (auto vec3ptr = std::get_if<glm::vec3>(&value)) {
            m_PackedParams.Mtl0.x = vec3ptr->x;
            m_PackedParams.Mtl0.y = vec3ptr->y;
            m_PackedParams.Mtl0.z = vec3ptr->z;
        }
        else {
            Logger::GetLogger()->warn("[Material: {}] Ambient must be glm::vec3!", m_Name);
        }
        break;
    }
    case MaterialParamType::Diffuse: { // Kd => Mtl1.xyz
        if (auto vec3ptr = std::get_if<glm::vec3>(&value)) {
            m_PackedParams.Mtl1.x = vec3ptr->x;
            m_PackedParams.Mtl1.y = vec3ptr->y;
            m_PackedParams.Mtl1.z = vec3ptr->z;
        }
        else {
            Logger::GetLogger()->warn("[Material: {}] Diffuse must be glm::vec3!", m_Name);
        }
        break;
    }
    case MaterialParamType::Specular: { // Ks => Mtl2.xyz
        if (auto vec3ptr = std::get_if<glm::vec3>(&value)) {
            m_PackedParams.Mtl2.x = vec3ptr->x;
            m_PackedParams.Mtl2.y = vec3ptr->y;
            m_PackedParams.Mtl2.z = vec3ptr->z;
        }
        else {
            Logger::GetLogger()->warn("[Material: {}] Specular must be glm::vec3!", m_Name);
        }
        break;
    }
    case MaterialParamType::Shininess: { // Ns => Mtl2.w
        if (auto fptr = std::get_if<float>(&value)) {
            m_PackedParams.Mtl2.w = *fptr;
        }
        else {
            Logger::GetLogger()->warn("[Material: {}] Shininess must be float!", m_Name);
        }
        break;
    }
    default:
        Logger::GetLogger()->warn("[Material: {}] Unhandled MaterialParamType {} in AssignToPackedParams.",
            m_Name, static_cast<int>(type));
        break;
    }

}

// --------------------------------------------------------
//  Set / Get texture
// --------------------------------------------------------
void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("[Material: {}] SetTexture: Null texture for type={}", m_Name, (int)type);
        return;
    }
    // Check if layout actually includes that texture
    if (!m_Layout.textures.count(type)) {
        Logger::GetLogger()->warn("[Material: {}] TextureType={} not in layout; ignoring.",
            m_Name, (int)type);
        return;
    }

    m_Textures[type] = texture;

    // set the bit in the usage mask
    m_TextureUsage |= (1 << static_cast<size_t>(type));
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
//  Custom param / custom texture
// --------------------------------------------------------
void Material::SetCustomParam(const std::string& paramName, UniformValue value)
{
    m_CustomParams[paramName] = std::move(value);
}

void Material::SetCustomTexture(const std::string& uniformName, const std::shared_ptr<ITexture>& texture)
{
    if (!texture) {
        Logger::GetLogger()->warn("[Material: {}] SetCustomTexture: Null texture for '{}'.", m_Name, uniformName);
        return;
    }
    m_CustomTextures[uniformName] = texture;
}

// --------------------------------------------------------
//  Bind / Unbind
// --------------------------------------------------------
void Material::Bind(const std::shared_ptr<BaseShader>& shader) const
{
    if (!shader) {
        Logger::GetLogger()->error("[Material: {}] Bind: No shader provided.", m_Name);
        return;
    }

    // 1) Send the bitmask
    shader->SetUniform("uMaterial_textureUsageFlags", (int)m_TextureUsage);

    // 2) Send the packed MTL parameters
    //    We'll call them Mtl0..Mtl3 in the shader
    shader->SetUniform("uMaterial.Mtl0", m_PackedParams.Mtl0);
    shader->SetUniform("uMaterial.Mtl1", m_PackedParams.Mtl1);
    shader->SetUniform("uMaterial.Mtl2", m_PackedParams.Mtl2);
    shader->SetUniform("uMaterial.Mtl3", m_PackedParams.Mtl3);

    // 3) Bind standard textures
    for (const auto& [texType, texPtr] : m_Textures) {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Bind(slot);

        std::string uniformName = GetTextureUniformName(texType);
        shader->SetUniform(uniformName, slot);
    }

    // 4) Bind custom textures
    //    We'll pick some starting slot after standard ones. 
    int customSlot = 10;
    for (const auto& [customName, texPtr] : m_CustomTextures) {
        if (!texPtr) continue;
        texPtr->Bind(customSlot);
        shader->SetUniform(customName, customSlot);
        ++customSlot;
    }

    // 5) Bind custom uniform params (int, float, vec*, mat*)
    for (const auto& [paramName, paramVal] : m_CustomParams) {
        // Use a lambda with std::visit to handle each type
        std::visit([&](auto&& arg) {
            shader->SetUniform(paramName, arg);
            }, paramVal);
    }

    // 5) Bind BRDFLut
    if (m_Layout.textures.contains(TextureType::BRDFLut)) {
        int slot = GetTextureBindingSlot(TextureType::BRDFLut);
        auto& textureManager = TextureManager::GetInstance();
        auto brdf = textureManager.GetTexture("brdfLut");
        brdf->Bind(slot);
    }
}

void Material::Unbind() const
{
    // For a simple scenario, just unbind the known texture slots
    for (const auto& [texType, texPtr] : m_Textures) {
        if (!texPtr) continue;
        int slot = GetTextureBindingSlot(texType);
        texPtr->Unbind(slot);
    }

    int customSlot = 10;
    for (const auto& [name, texPtr] : m_CustomTextures) {
        if (!texPtr) continue;
        texPtr->Unbind(customSlot);
        ++customSlot;
    }
}