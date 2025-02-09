#include "Resources/MeshManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/AllMeshes.h"

MeshManager& MeshManager::GetInstance() {
    static MeshManager instance;
    return instance;
}

std::shared_ptr<Mesh> MeshManager::GetMesh(std::string_view meshName) {
    std::string meshNameStr(meshName);
    auto it = m_Meshes.find(meshNameStr);
    if (it != m_Meshes.end()) {
        return it->second;
    }

    // If not found, create mesh based on known mesh names
    std::shared_ptr<Mesh> mesh;
    if (meshName == "cube") {
        mesh = std::make_shared<Cube>();
    }
    else if (meshName == "sphere") {
        mesh = std::make_shared<Sphere>();
    }
    else if (meshName == "lightsphere") {
        mesh = std::make_shared<LightSphere>();
    }
    else if (meshName == "quad") {
        mesh = std::make_shared<Quad>();
    }
    else if (meshName == "floor") {
        mesh = std::make_shared<Floor>();
    }
    else if (meshName == "terrain") {
        mesh = std::make_shared<TerrainMesh>();
    }
    else {
        Logger::GetLogger()->warn("Mesh '{}' not recognized. Returning nullptr.", meshNameStr);
        return nullptr;
    }

    m_Meshes[meshNameStr] = mesh;
    return mesh;
}

bool MeshManager::DeleteMesh(std::string_view meshName) {
    return m_Meshes.erase(std::string(meshName)) > 0;
}

std::shared_ptr<Graphics::MeshBuffer> MeshManager::GetMeshBuffer(std::string_view meshName, const MeshLayout& layout) {
    MeshKey key = { std::string(meshName), layout };

    auto it = m_MeshBuffers.find(key);
    if (it != m_MeshBuffers.end()) {
        return it->second;
    }

    auto mesh = GetMesh(meshName);
    if (!mesh) {
        Logger::GetLogger()->error("Mesh '{}' not found. Cannot create MeshBuffer.", meshName);
        return nullptr;
    }

    auto meshBuffer = std::make_shared<Graphics::MeshBuffer>(*mesh, layout);
    m_MeshBuffers[key] = meshBuffer;
    return meshBuffer;
}

bool MeshManager::DeleteMeshBuffer(std::string_view meshName, const MeshLayout& layout) {
    MeshKey key = { std::string(meshName), layout };
    return m_MeshBuffers.erase(key) > 0;
}

void MeshManager::Clear() {
    m_MeshBuffers.clear();
    m_Meshes.clear();
    Logger::GetLogger()->info("MeshManager cleared all meshes and buffers.");
}