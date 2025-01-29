#pragma once

#include "Mesh.h"

/**
 * @brief A simple mesh representing a 1x1x1 cube centered at the origin.
 *        Extends Mesh to pre-populate vertex/index data.
 */
struct Cube : public Mesh {
    Cube();
};