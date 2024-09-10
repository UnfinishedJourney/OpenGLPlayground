#pragma once

#include "Model.h"
#include "Material.h"
#include "Shader.h"

/*class ResourceManager {
public:
    std::shared_ptr<Mesh> GetMesh(const std::string& meshName) {
        // Check if the mesh already exists
        if (m_Meshes.find(meshName) != m_Meshes.end()) {
            return m_Meshes[meshName]; // Return the existing mesh
        }
        // Otherwise, load it (placeholder loading logic)
        std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();
        m_Meshes[meshName] = newMesh;
        return newMesh;
    }

    // Load or retrieve an existing material
    std::shared_ptr<Material> GetMaterial(const std::string& materialName) {
        if (m_Materials.find(materialName) != m_Materials.end()) {
            return m_Materials[materialName]; // Return the existing material
        }
        // Otherwise, create/load a new material (placeholder)
        std::shared_ptr<Material> newMaterial = std::make_shared<Material>();
        m_Materials[materialName] = newMaterial;
        return newMaterial;
    }

    // Unload all resources
    void UnloadAll() {
        m_Meshes.clear();
        m_MeshComponents.clear();
        m_Materials.clear();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<std::string, std::shared_ptr<MeshComponent>> m_MeshComponents;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};*/