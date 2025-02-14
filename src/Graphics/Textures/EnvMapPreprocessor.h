#pragma once
#include <array>
#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include "Bitmap.h"

//https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook

namespace graphics {

    class EnvMapPreprocessor {
    public:
        Bitmap LoadTexture(const std::filesystem::path& texturePath) const;
        Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& input) const;
        Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& input) const;
        Bitmap ComputeIrradianceEquirect(const Bitmap& inEquirect, int outW, int outH, int samples) const;
        //need to fix
        std::vector<Bitmap> ComputePrefilteredCubemap(const Bitmap& inEquirect, int baseFaceSize, int numSamples) const;
    };

} // namespace graphics