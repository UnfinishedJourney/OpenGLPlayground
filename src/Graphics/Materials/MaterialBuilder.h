#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Material.h"

/**
 * @brief A fluent builder for constructing Materials.
 */
class MaterialBuilder
{
public:
    MaterialBuilder() = default;

    MaterialBuilder& WithName(const std::string& name)
    {
        m_Name = name;
        return *this;
    }

    MaterialBuilder& WithID(MaterialID id)
    {
        m_ID = id;
        return *this;
    }

    MaterialBuilder& WithLayout(const MaterialLayout& layout)
    {
        m_Layout = layout;
        return *this;
    }

    MaterialBuilder& WithParam(MaterialParamType type, const UniformValue& value)
    {
        m_Params[type] = value;
        // We also track that the layout should have this param
        m_Layout.params.insert(type);
        return *this;
    }

    MaterialBuilder& WithCustomParam(const std::string& name, const UniformValue& value)
    {
        m_CustomParams[name] = value;
        return *this;
    }

    MaterialBuilder& WithTexture(TextureType type, const std::shared_ptr<ITexture>& texture)
    {
        m_Textures[type] = texture;
        m_Layout.textures.insert(type);
        return *this;
    }

    MaterialBuilder& WithCustomTexture(const std::string& name, const std::shared_ptr<ITexture>& texture)
    {
        m_CustomTextures[name] = texture;
        return *this;
    }

    /**
     * @brief Produce the final Material.
     */
    std::shared_ptr<Material> Build() const
    {
        auto mat = std::make_shared<Material>(m_Layout);
        mat->SetName(m_Name);
        mat->SetID(m_ID);
        //mat->SetLayout(m_Layout);

        // Set standard params
        for (auto& [type, value] : m_Params) {
            mat->SetParam(type, value);
        }
        // Set custom params
        for (auto& [name, value] : m_CustomParams) {
            mat->SetCustomParam(name, value);
        }
        // Set textures
        for (auto& [type, texPtr] : m_Textures) {
            mat->SetTexture(type, texPtr);
        }
        // Custom textures
        for (auto& [name, texPtr] : m_CustomTextures) {
            mat->SetCustomTexture(name, texPtr);
        }

        return mat;
    }

private:
    std::string m_Name;
    MaterialID m_ID = -1;
    MaterialLayout m_Layout;

    std::unordered_map<MaterialParamType, UniformValue>         m_Params;
    std::unordered_map<std::string, UniformValue>               m_CustomParams;
    std::unordered_map<TextureType, std::shared_ptr<ITexture>>  m_Textures;
    std::unordered_map<std::string, std::shared_ptr<ITexture>>  m_CustomTextures;
};