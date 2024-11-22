#pragma once

#include "Mesh.h"
#include <glm/glm.hpp>

struct Sphere : public Mesh {
public:
    Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
};

struct LightSphere : public Sphere {
public:
    LightSphere() : Sphere(0.2f, 20, 10) {}
};