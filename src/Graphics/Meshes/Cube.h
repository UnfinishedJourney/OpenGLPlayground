#pragma once

#include "Mesh.h"

namespace Graphics {

    /**
     * @brief Predefined 1x1x1 cube centered at the origin.
     */
    struct Cube : public Mesh {
        Cube();
    };

} // namespace Graphics