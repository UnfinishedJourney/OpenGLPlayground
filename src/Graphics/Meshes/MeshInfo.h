#pragma once

#include <memory>
#include "Graphics/Meshes/Mesh.h"

/**
 * @brief Basic container referencing a Mesh plus optional material index.
 */
struct MeshInfo
{
    std::shared_ptr<Mesh> mesh;
    int                   materialIndex = -1;
    // If you have additional data (e.g., textures) store them here
};