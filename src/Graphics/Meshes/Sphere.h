#pragma once

#include "Mesh.h"

/**
 * @brief A UV sphere of a given radius, sectorCount, stackCount.
 */
struct Sphere : public Mesh {
    Sphere(float radius = 1.0f,
        unsigned int sectorCount = 36,
        unsigned int stackCount = 18);
};

/**
 * @brief A small "light sphere" that defaults to radius=0.1, etc.
 */
struct LightSphere : public Sphere {
    LightSphere()
        : Sphere(0.1f, 20, 10)
    {}
};