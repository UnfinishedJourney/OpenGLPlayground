#pragma once
#include "Graphics/Meshes/Mesh.h"

struct Sphere : Mesh {
public:
    Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
};

struct LightSphere : Sphere {
public:
    LightSphere() : Sphere(0.2f, 20, 10) {}
};