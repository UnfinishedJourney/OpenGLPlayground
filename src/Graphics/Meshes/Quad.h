#pragma once

#include "Mesh.h"

namespace graphics {

    /**
     * @brief A 2D quad in the XY plane (Z = 0) with corners at (-1, -1) and (1, 1).
     */
    struct Quad : public Mesh {
        Quad();
    };

    /**
     * @brief A simple floor mesh.
     */
    struct Floor : public Mesh {
        Floor();
    };

} // namespace graphics