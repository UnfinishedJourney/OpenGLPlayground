#include "MeshManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/AllMeshes.h"  // Assumes this header includes all your mesh types (Cube, Sphere, Quad, etc.)

namespace Graphics {

    MeshManager& MeshManager::GetInstance() {
        static MeshManager instance;
        return instance;
    }

    std::shared_ptr<Mesh> MeshManager::GetMesh(std::string_view meshName) {
        std::string meshNameStr(meshName);
        auto it = meshes_.find(meshNameStr);
        if (it != meshes_.end()) {
            return it->second;
        }

        // If not found, create a mesh based on known names.
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
        //else if (meshName == "terrain") {
        //    // Assuming TerrainMesh is defined somewhere.
        //    mesh = std::make_shared<TerrainMesh>();
        //}
        else {
            Logger::GetLogger()->warn("Mesh '{}' not recognized. Returning nullptr.", meshNameStr);
            return nullptr;
        }

        meshes_[meshNameStr] = mesh;
        return mesh;
    }

    bool MeshManager::DeleteMesh(std::string_view meshName) {
        return meshes_.erase(std::string(meshName)) > 0;
    }

    std::shared_ptr<MeshBuffer> MeshManager::GetMeshBuffer(std::string_view meshName, const MeshLayout& layout) {
        MeshKey key = { std::string(meshName), layout };
        auto it = meshBuffers_.find(key);
        if (it != meshBuffers_.end()) {
            return it->second;
        }

        auto mesh = GetMesh(meshName);
        if (!mesh) {
            Logger::GetLogger()->error("Mesh '{}' not found. Cannot create MeshBuffer.", meshName);
            return nullptr;
        }

        auto meshBuffer = std::make_shared<MeshBuffer>(*mesh, layout);
        meshBuffers_[key] = meshBuffer;
        return meshBuffer;
    }

    bool MeshManager::DeleteMeshBuffer(std::string_view meshName, const MeshLayout& layout) {
        MeshKey key = { std::string(meshName), layout };
        return meshBuffers_.erase(key) > 0;
    }

    void MeshManager::Clear() {
        meshBuffers_.clear();
        meshes_.clear();
        Logger::GetLogger()->info("MeshManager cleared all meshes and buffers.");
    }

} // namespace Graphics