#include "Graphics/Meshes/Quad.h"

Quad::Quad()
{
    positions = {
        {-1.0f,  1.0f, 0.0f}, // Top-left
        {-1.0f, -1.0f, 0.0f}, // Bottom-left
        { 1.0f, -1.0f, 0.0f}, // Bottom-right
        { 1.0f,  1.0f, 0.0f}  // Top-right
    };

    uvs[TextureType::Albedo] = {
        {0.0f, 1.0f}, // Top-left
        {0.0f, 0.0f}, // Bottom-left
        {1.0f, 0.0f}, // Bottom-right
        {1.0f, 1.0f}  // Top-right
    };

    indices = {
        0, 1, 2, // First Triangle
        2, 3, 0  // Second Triangle
    };
}