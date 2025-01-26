#include "MaterialManager.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"
// #include "Resources/TextureManager.h" // If needed

MaterialManager& MaterialManager::GetInstance()
{
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterialByName(const std::string& name) const
{
    auto it = m_NameToIndex.find(name);
    if (it != m_NameToIndex.end()) {
        return m_Materials[it->second].material;
    }
    Logger::GetLogger()->warn("Material '{}' not found.", name);
    return nullptr;
}

void MaterialManager::AddMaterial(const std::shared_ptr<Material>& material)
{
    if (!material) {
        Logger::GetLogger()->error("AddMaterial: Material pointer is null.");
        return;
    }
    
    auto name = material->GetName();
    auto layout = material->GetLayout();

    auto it = m_NameToIndex.find(name);
    if (it != m_NameToIndex.end()) {
        Logger::GetLogger()->warn("Trying to override a material");
        //add proper material removal
    }

    ++m_LastId;
    material->SetID(m_LastId);
    size_t newIndex = m_Materials.size();
    m_Materials.emplace_back(name, m_LastId, layout, material);
    m_NameToIndex[name] = newIndex;
    m_IDToIndex[m_LastId] = newIndex;
}

std::shared_ptr<Material> MaterialManager::GetMaterialByID(MaterialID id) const
{
    auto it = m_IDToIndex.find(id);
    if (it != m_IDToIndex.end()) {
        return m_Materials[it->second].material;
    }
    Logger::GetLogger()->warn("Material with ID {} not found.", id);
    return nullptr;
}



// --- Binding ---

void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader)
{
    auto mat = GetMaterialByName(name);
    if (!mat) {
        Logger::GetLogger()->error("Cannot bind unknown material '{}'.", name);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = name;
}

void MaterialManager::BindMaterial(int id, const std::shared_ptr<BaseShader>& shader)
{
    auto mat = GetMaterialByID(id);
    if (!mat) {
        Logger::GetLogger()->error("Cannot bind unknown material '{}'.", id);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = mat->GetName();
}

void MaterialManager::UnbindMaterial()
{
    if (!m_CurrentlyBoundMaterial.empty()) {
        auto mat = GetMaterialByName(m_CurrentlyBoundMaterial);
        if (mat) mat->Unbind();
        m_CurrentlyBoundMaterial.clear();
    }
}

// --- Utility ---

void MaterialManager::InitializeStandardMaterials()
{
//    // Example: Gold
//    MaterialLayout goldLayout{
//        { MaterialParamType::Ambient, MaterialParamType::Diffuse,
//          MaterialParamType::Specular, MaterialParamType::Shininess },
//        {} // textures if any
//    };
//
//    // Construct via builder for clarity
//    auto gold = MaterialBuilder()
//        .WithName("Gold")
//        .WithParam(MaterialParamType::Ambient, glm::vec3(0.24725f, 0.1995f, 0.0745f))
//        .WithParam(MaterialParamType::Diffuse, glm::vec3(0.75164f, 0.60648f, 0.22648f))
//        .WithParam(MaterialParamType::Specular, glm::vec3(0.628281f, 0.555802f, 0.366065f))
//        .WithParam(MaterialParamType::Shininess, 51.2f)
//        .WithLayout(goldLayout)
//        .Build();
//
//    AddMaterial(gold);
//
//    // Example: Silver
//    MaterialLayout silverLayout{
//        { MaterialParamType::Ambient, MaterialParamType::Diffuse,
//          MaterialParamType::Specular, MaterialParamType::Shininess },
//        {}
//    };
//
//    auto silver = MaterialBuilder()
//        .WithName("Silver")
//        .WithParam(MaterialParamType::Ambient, glm::vec3(0.19225f))
//        .WithParam(MaterialParamType::Diffuse, glm::vec3(0.50754f))
//        .WithParam(MaterialParamType::Specular, glm::vec3(0.508273f))
//        .WithParam(MaterialParamType::Shininess, 51.2f)
//        .WithLayout(silverLayout)
//        .Build();
//
//    AddMaterial(silver);
}

std::vector<std::string> MaterialManager::GetMaterialNames() const
{
    std::vector<std::string> names;
    names.reserve(m_Materials.size());
    for (auto& entry : m_Materials) {
        names.push_back(entry.name);
    }
    return names;
}
