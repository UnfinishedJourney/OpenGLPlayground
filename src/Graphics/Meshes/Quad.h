#pragma once

#include "Mesh.h"

/**
 * @brief A simple 2D quad in the XY plane (Z=0),
 *        with corners at (-1, -1), (1, 1).
 */
struct Quad : public Mesh {
    Quad();
};

struct Floor : public Mesh {
    Floor();
};