#pragma once

#include "Mesh.h"

namespace graphics {

    /**
     * @brief Predefined 1x1x1 cube centered at the origin.
     */
    struct Cube : public Mesh {
        Cube();
    };

} // namespace graphics