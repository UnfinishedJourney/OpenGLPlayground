#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/MeshLayout.h"

class MeshManager {
public:
    static MeshManager& GetInstance();

    std::shared_ptr<Mesh> GetMesh(std::string_view meshName);
    bool DeleteMesh(std::string_view meshName);

    std::shared_ptr<Graphics::MeshBuffer> GetMeshBuffer(std::string_view meshName, const MeshLayout& layout);
    bool DeleteMeshBuffer(std::string_view meshName, const MeshLayout& layout);

    void Clear();

private:
    MeshManager() = default;
    ~MeshManager() = default;

    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;

    struct MeshKey {
        std::string name;
        MeshLayout meshLayout;

        bool operator==(const MeshKey& other) const {
            return name == other.name && meshLayout == other.meshLayout;
        }
    };

    struct MeshKeyHash {
        std::size_t operator()(const MeshKey& key) const {
            std::size_t h1 = std::hash<std::string>{}(key.name);
            std::size_t h2 = std::hash<MeshLayout>{}(key.meshLayout);
            return h1 ^ (h2 << 1);
        }
    };

    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<MeshKey, std::shared_ptr<Graphics::MeshBuffer>, MeshKeyHash> m_MeshBuffers;
};