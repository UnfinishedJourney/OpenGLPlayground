#pragma once

#include <array>
#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

#include "Bitmap.h"

//https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook/



//need to fix prefiltered map, maybe save cross too
namespace graphics {

    /**
     * @brief Provides methods to convert and process environment maps.
     *
     * Functions include conversion between equirectangular maps and vertical cross/cubemap layouts,
     * as well as irradiance and prefiltered cubemap computation.
     */
    class EnvMapPreprocessor {
    public:

        [[nodiscard]] Bitmap LoadTexture(const std::filesystem::path& texturePath) const;

        [[nodiscard]] Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& input) const;

        [[nodiscard]] Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& input) const;

        /**
         * @brief Computes an irradiance map from an equirectangular environment map.
         *
         * @param inEquirect Input equirectangular Bitmap.
         * @param outWidth   Output image width.
         * @param outHeight  Output image height.
         * @param samples    Number of samples per pixel.
         * @return Irradiance Bitmap.
         */
        [[nodiscard]] Bitmap ComputeIrradianceEquirect(const Bitmap& inEquirect,
            int outWidth, int outHeight,
            int samples) const;

        /**
         * @brief Computes a prefiltered cubemap for specular IBL from an equirectangular map.
         *
         * @param inEquirect Input equirectangular Bitmap.
         * @param baseFaceSize Base size for cubemap faces.
         * @param numSamples Number of samples per pixel.
         * @return A vector of Bitmaps for each mip–level (each with depth == 6).
         */
        [[nodiscard]] std::vector<Bitmap> ComputePrefilteredCubemap(const Bitmap& inEquirect,
            int baseFaceSize,
            int numSamples) const;
    };

} // namespace graphics