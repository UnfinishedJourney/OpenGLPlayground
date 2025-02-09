#include "Material.h"
#include "Graphics/Textures/TextureManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace Graphics {

    // Helpers for mapping TextureType to uniform names and binding slots.
    static std::string GetTextureUniformName(TextureType type) {
        switch (type) {
        case TextureType::Albedo:           return "uTexAlbedo";
        case TextureType::Normal:           return "uTexNormal";
        case TextureType::MetalRoughness:   return "uTexMetalRoughness";
        case TextureType::AO:               return "uTexAO";
        case TextureType::Emissive:         return "uTexEmissive";
        case TextureType::Ambient:          return "uTexAmbient";
        case TextureType::Height:           return "uTexHeight";
        case TextureType::BRDFLut:          return "uTexBRDFLUT";
        default:                            return "uTexUnknown";
        }
    }

    static int GetTextureBindingSlot(TextureType type) {
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
        default:                            return 11;
        }
    }

    // ---------------- Material ----------------
    Material::Material(const MaterialLayout& layout)
        : m_Layout(layout) {}

    void Material::SetName(const std::string& name) {
        m_Name = name;
    }

    const std::string& Material::GetName() const {
        return m_Name;
    }

    void Material::SetID(int id) {
        m_ID = id;
    }

    int Material::GetID() const {
        return m_ID;
    }

    const MaterialLayout& Material::GetLayout() const {
        return m_Layout;
    }

    void Material::AssignToPackedParams(MaterialParamType type, const UniformValue& value) {
        switch (type) {
        case MaterialParamType::Ambient: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                m_PackedParams.Mtl0.x = vec3Ptr->x;
                m_PackedParams.Mtl0.y = vec3Ptr->y;
                m_PackedParams.Mtl0.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Ambient must be glm::vec3!", m_Name);
            }
            break;
        }
        case MaterialParamType::Diffuse: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                m_PackedParams.Mtl1.x = vec3Ptr->x;
                m_PackedParams.Mtl1.y = vec3Ptr->y;
                m_PackedParams.Mtl1.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Diffuse must be glm::vec3!", m_Name);
            }
            break;
        }
        case MaterialParamType::Specular: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                m_PackedParams.Mtl2.x = vec3Ptr->x;
                m_PackedParams.Mtl2.y = vec3Ptr->y;
                m_PackedParams.Mtl2.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Specular must be glm::vec3!", m_Name);
            }
            break;
        }
        case MaterialParamType::Shininess: {
            if (auto fPtr = std::get_if<float>(&value)) {
                m_PackedParams.Mtl2.w = *fPtr;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Shininess must be float!", m_Name);
            }
            break;
        }
        default:
            Logger::GetLogger()->warn("[Material: {}] Unhandled MaterialParamType in AssignToPackedParams.", m_Name);
            break;
        }
    }

    void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture) {
        if (!texture) {
            Logger::GetLogger()->warn("[Material: {}] SetTexture: Null texture for type={}", m_Name, static_cast<int>(type));
            return;
        }
        if (m_Layout.textures_.find(type) == m_Layout.textures_.end()) {
            Logger::GetLogger()->warn("[Material: {}] TextureType={} not in layout; ignoring.", m_Name, static_cast<int>(type));
            return;
        }
        m_Textures[type] = texture;
        m_TextureUsage |= (1 << static_cast<size_t>(type));
    }

    std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const {
        auto it = m_Textures.find(type);
        return (it != m_Textures.end()) ? it->second : nullptr;
    }

    void Material::SetCustomParam(const std::string& paramName, UniformValue value) {
        m_CustomParams[paramName] = std::move(value);
    }

    void Material::SetCustomTexture(const std::string& uniformName, const std::shared_ptr<ITexture>& texture) {
        if (!texture) {
            Logger::GetLogger()->warn("[Material: {}] SetCustomTexture: Null texture for '{}'.", m_Name, uniformName);
            return;
        }
        m_CustomTextures[uniformName] = texture;
    }

    void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
        if (!shader) {
            Logger::GetLogger()->error("[Material: {}] Bind: No shader provided.", m_Name);
            return;
        }
        // 1) Send texture usage bitmask.
        shader->SetUniform("uMaterial_textureUsageFlags", static_cast<int>(m_TextureUsage));
        // 2) Send packed parameters.
        shader->SetUniform("uMaterial.Mtl0", m_PackedParams.Mtl0);
        shader->SetUniform("uMaterial.Mtl1", m_PackedParams.Mtl1);
        shader->SetUniform("uMaterial.Mtl2", m_PackedParams.Mtl2);
        shader->SetUniform("uMaterial.Mtl3", m_PackedParams.Mtl3);
        // 3) Bind standard textures.
        for (const auto& [texType, texPtr] : m_Textures) {
            if (!texPtr)
                continue;
            int slot = GetTextureBindingSlot(texType);
            texPtr->Bind(static_cast<uint32_t>(slot));
            shader->SetUniform(GetTextureUniformName(texType), slot);
        }
        // 4) Bind custom textures.
        int customSlot = 10;
        for (const auto& [uniformName, texPtr] : m_CustomTextures) {
            if (!texPtr)
                continue;
            texPtr->Bind(static_cast<uint32_t>(customSlot));
            shader->SetUniform(uniformName, customSlot);
            ++customSlot;
        }
        // 5) Bind custom parameters.
        for (const auto& [paramName, paramVal] : m_CustomParams) {
            std::visit([&](auto&& arg) {
                shader->SetUniform(paramName, arg);
                }, paramVal);
        }
        // 6) Optionally, bind the BRDF LUT if defined in the layout.
        if (m_Layout.textures_.find(TextureType::BRDFLut) != m_Layout.textures_.end()) {
            int slot = GetTextureBindingSlot(TextureType::BRDFLut);
            auto& texManager = TextureManager::GetInstance();
            auto brdfTex = texManager.GetTexture("brdfLut");
            if (brdfTex)
                brdfTex->Bind(static_cast<uint32_t>(slot));
        }
    }

    void Material::Unbind() const {
        for (const auto& [texType, texPtr] : m_Textures) {
            if (!texPtr)
                continue;
            int slot = GetTextureBindingSlot(texType);
            texPtr->Unbind(static_cast<uint32_t>(slot));
        }
        int customSlot = 10;
        for (const auto& [name, texPtr] : m_CustomTextures) {
            if (!texPtr)
                continue;
            texPtr->Unbind(static_cast<uint32_t>(customSlot));
            ++customSlot;
        }
    }

} // namespace Graphics