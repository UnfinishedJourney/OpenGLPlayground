#pragma once

#include <memory>
#include "Mesh.h"

namespace graphics {

    /**
     * @brief Holds a reference to a Mesh plus an associated material index.
     *
     * When rendering, the material identified by materialIndex_ should be bound.
     */
    struct MeshInfo {
        std::shared_ptr<Mesh> mesh_;
        int materialIndex_ = -1;
    };

} // namespace graphics