#pragma once

#include <Graphics/Materials/Material.h>
#include "Graphics/Textures/Texture2D.h"
#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

class MaterialManager {
public:
    MaterialManager() = default;
    ~MaterialManager() = default;

    std::shared_ptr<Material> GetMaterial(const std::string& materialName);
    std::shared_ptr<Texture2D> GetTexture(const std::string& textureName);
    void AddMaterial(std::string name, std::shared_ptr<Material> material);
    void BindMaterial(const std::string& name, std::shared_ptr<BaseShader> shader);
    void UnbindMaterial(const std::string& name);

private:
    std::string m_CurrentlyBoundMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
};