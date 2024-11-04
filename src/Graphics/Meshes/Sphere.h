#pragma once
#include "Graphics/Meshes/Mesh.h"

//maybe potentially need isosphere

struct Sphere : Mesh {
public:
    /*
     * radius The radius of the sphere.
     * sectorCount Number of longitudinal slices.
     * stackCount Number of latitudinal stacks.
     */

    Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
};