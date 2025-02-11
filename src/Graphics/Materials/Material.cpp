#include "Material.h"
#include "Graphics/Textures/TextureManager.h"
#include "Utilities/Logger.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <utility>

namespace Graphics {

    // Helper functions for mapping texture types to uniform names and binding slots.
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

    // ---------- Material Implementation ----------

    Material::Material(const MaterialLayout& layout)
        : layout_(layout)
    {}

    void Material::SetName(const std::string& name) {
        name_ = name;
    }

    const std::string& Material::GetName() const {
        return name_;
    }

    void Material::SetID(std::size_t id) {
        id_ = id;
    }

    std::size_t Material::GetID() const {
        return id_;
    }

    const MaterialLayout& Material::GetLayout() const {
        return layout_;
    }

    void Material::AssignToPackedParams(MaterialParamType type, const UniformValue& value) {
        switch (type) {
        case MaterialParamType::Ambient: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                packed_params_.mtl0_.x = vec3Ptr->x;
                packed_params_.mtl0_.y = vec3Ptr->y;
                packed_params_.mtl0_.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Ambient must be glm::vec3!", name_);
            }
            break;
        }
        case MaterialParamType::Diffuse: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                packed_params_.mtl1_.x = vec3Ptr->x;
                packed_params_.mtl1_.y = vec3Ptr->y;
                packed_params_.mtl1_.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Diffuse must be glm::vec3!", name_);
            }
            break;
        }
        case MaterialParamType::Specular: {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value)) {
                packed_params_.mtl2_.x = vec3Ptr->x;
                packed_params_.mtl2_.y = vec3Ptr->y;
                packed_params_.mtl2_.z = vec3Ptr->z;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Specular must be glm::vec3!", name_);
            }
            break;
        }
        case MaterialParamType::Shininess: {
            if (auto fPtr = std::get_if<float>(&value)) {
                packed_params_.mtl2_.w = *fPtr;
            }
            else {
                Logger::GetLogger()->warn("[Material: {}] Shininess must be float!", name_);
            }
            break;
        }
        default:
            Logger::GetLogger()->warn("[Material: {}] Unhandled MaterialParamType in AssignToPackedParams.", name_);
            break;
        }
    }

    void Material::SetTexture(TextureType type, const std::shared_ptr<ITexture>& texture) {
        if (!texture) {
            Logger::GetLogger()->warn("[Material: {}] SetTexture: Null texture for type={}", name_, static_cast<int>(type));
            return;
        }
        if (!layout_.HasTexture(type)) {
            Logger::GetLogger()->warn("[Material: {}] TextureType={} not in layout; ignoring.", name_, static_cast<int>(type));
            return;
        }
        textures_[type] = texture;
        texture_usage_ |= (1 << static_cast<size_t>(type));
    }

    std::shared_ptr<ITexture> Material::GetTexture(TextureType type) const {
        auto it = textures_.find(type);
        return (it != textures_.end()) ? it->second : nullptr;
    }

    void Material::SetCustomParam(const std::string& param_name, UniformValue value) {
        custom_params_[param_name] = std::move(value);
    }

    void Material::SetCustomTexture(const std::string& uniform_name, const std::shared_ptr<ITexture>& texture) {
        if (!texture) {
            Logger::GetLogger()->warn("[Material: {}] SetCustomTexture: Null texture for '{}'.", name_, uniform_name);
            return;
        }
        custom_textures_[uniform_name] = texture;
    }

    void Material::Bind(const std::shared_ptr<BaseShader>& shader) const {
        if (!shader) {
            Logger::GetLogger()->error("[Material: {}] Bind: No shader provided.", name_);
            return;
        }
        // 1) Set texture usage bitmask.
        shader->SetUniform("uMaterial_textureUsageFlags", static_cast<int>(texture_usage_));
        // 2) Set packed material parameters.
        shader->SetUniform("uMaterial.Mtl0", packed_params_.mtl0_);
        shader->SetUniform("uMaterial.Mtl1", packed_params_.mtl1_);
        shader->SetUniform("uMaterial.Mtl2", packed_params_.mtl2_);
        shader->SetUniform("uMaterial.Mtl3", packed_params_.mtl3_);
        // 3) Bind standard textures.
        for (const auto& [texType, texPtr] : textures_) {
            if (!texPtr)
                continue;
            int slot = GetTextureBindingSlot(texType);
            texPtr->Bind(static_cast<uint32_t>(slot));
            shader->SetUniform(GetTextureUniformName(texType), slot);
        }
        // 4) Bind custom textures.
        int customSlot = 10;
        for (const auto& [uniformName, texPtr] : custom_textures_) {
            if (!texPtr)
                continue;
            texPtr->Bind(static_cast<uint32_t>(customSlot));
            shader->SetUniform(uniformName, customSlot);
            ++customSlot;
        }
        // 5) Bind custom parameters.
        for (const auto& [paramName, paramVal] : custom_params_) {
            std::visit([&](auto&& arg) {
                shader->SetUniform(paramName, arg);
                }, paramVal);
        }
        // 6) Optionally bind the BRDF LUT if defined in the layout.
        if (layout_.HasTexture(TextureType::BRDFLut)) {
            int slot = GetTextureBindingSlot(TextureType::BRDFLut);
            auto& texManager = TextureManager::GetInstance();
            auto brdfTex = texManager.GetTexture("brdfLut");
            if (brdfTex)
                brdfTex->Bind(static_cast<uint32_t>(slot));
        }
    }

    void Material::Unbind() const {
        for (const auto& [texType, texPtr] : textures_) {
            if (!texPtr)
                continue;
            int slot = GetTextureBindingSlot(texType);
            texPtr->Unbind(static_cast<uint32_t>(slot));
        }
        int customSlot = 10;
        for (const auto& [name, texPtr] : custom_textures_) {
            if (!texPtr)
                continue;
            texPtr->Unbind(static_cast<uint32_t>(customSlot));
            ++customSlot;
        }
    }

} // namespace Graphics