#pragma once

#include <memory>

#include "Mesh.h"

/**
 * @brief A small struct referencing a Mesh plus an associated material index.
 *
 * If your system uses MaterialID (int), store that here so you know
 * which Material to bind for rendering.
 */
struct MeshInfo {
    std::shared_ptr<Mesh> mesh;
    int                   materialIndex = -1;
    // Additional data if desired (e.g. transform, layer, etc.)
};