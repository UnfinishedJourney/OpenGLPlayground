#pragma once
#include <array>
#include <filesystem>
#include <vector>
#include "Bitmap.h"
#include <glm/glm.hpp>

//modified
//https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook

namespace graphics {

    class EnvMapPreprocessor {
    public:
        // Convert an equirectangular map into a vertical cross image.
        void SaveAsVerticalCross(const std::filesystem::path& texturePath,
            const std::filesystem::path& outPath) const;

        // Load an HDR texture from file into a Bitmap.
        Bitmap LoadTexture(const std::filesystem::path& texturePath) const;

        // Convert an equirectangular map to vertical cross format.
        Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& input) const;

        // Slice a vertical cross image into cube map faces (returned as a Bitmap with depth==6).
        Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& input) const;

        // Compute an irradiance map from an equirectangular environment.
        Bitmap ComputeIrradianceEquirect(const Bitmap& inEquirect, int outWidth, int outHeight, int samples) const;

        // Save the image as an HDR file.
        void SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) const;

        // Save the image as an LDR (PNG) file.
        void SaveAsLDR(const Bitmap& image, const std::filesystem::path& outPath) const;

        // Save the 6 cube faces (contained in a cubemap Bitmap) as LDR images.
        void SaveFacesToDiskLDR(const Bitmap& cubeMap,
            const std::array<std::filesystem::path, 6>& facePaths,
            const std::string& prefix) const;

        // Compute a prefiltered cubemap (for specular IBL) from an equirectangular map.
        std::vector<Bitmap> ComputePrefilteredCubemap(const Bitmap& inEquirect, int baseFaceSize, int numSamples) const; //does not work yet
    };

} // namespace graphics