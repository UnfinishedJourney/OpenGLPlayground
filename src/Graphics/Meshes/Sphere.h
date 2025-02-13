#pragma once

#include "Mesh.h"

namespace graphics {

    /**
     * @brief Generates a UV sphere mesh with a given radius, sector count, and stack count.
     */
    struct Sphere : public Mesh {
        Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
    };

    /**
     * @brief A small sphere intended for use as a light source.
     */
    struct LightSphere : public Sphere {
        LightSphere() : Sphere(0.1f, 20, 10) {}
    };

} // namespace graphics