#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/MeshLayout.h"

namespace graphics {

    /**
     * @brief Manages meshes and mesh buffers.
     *
     * Provides a singleton lookup to create, retrieve, and delete meshes and mesh buffers.
     */
    class MeshManager {
    public:
        static MeshManager& GetInstance();

        std::shared_ptr<Mesh> GetMesh(std::string_view meshName);
        bool DeleteMesh(std::string_view meshName);

        std::shared_ptr<MeshBuffer> GetMeshBuffer(std::string_view meshName, const MeshLayout& layout);
        bool DeleteMeshBuffer(std::string_view meshName, const MeshLayout& layout);

        void Clear();

    private:
        MeshManager() = default;
        ~MeshManager() = default;
        MeshManager(const MeshManager&) = delete;
        MeshManager& operator=(const MeshManager&) = delete;

        struct MeshKey {
            std::string name_;
            MeshLayout meshLayout_;

            bool operator==(const MeshKey& other) const {
                return name_ == other.name_ && meshLayout_ == other.meshLayout_;
            }
        };

        struct MeshKeyHash {
            std::size_t operator()(const MeshKey& key) const {
                std::size_t h1 = std::hash<std::string>{}(key.name_);
                std::size_t h2 = std::hash<MeshLayout>{}(key.meshLayout_);
                return h1 ^ (h2 << 1);
            }
        };

        std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes_;
        std::unordered_map<MeshKey, std::shared_ptr<MeshBuffer>, MeshKeyHash> meshBuffers_;
    };

} // namespace graphics