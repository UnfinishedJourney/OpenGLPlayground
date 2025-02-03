#pragma once

#include <glm/glm.hpp>
#include <string>
#include <filesystem>
#include "Bitmap.h"

class EnvMapPreprocessor
{
public:
    void SaveAsVerticalCross(const std::filesystem::path& texturePath, const std::filesystem::path& outPath) const;
    Bitmap LoadTexture(const std::filesystem::path& texturePath) const;
    Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& b) const;
    Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) const;

    inline Bitmap ConvertEquirectangularMapToCubeMapFaces(const Bitmap& b) const {
        return ConvertVerticalCrossToCubeMapFaces(ConvertEquirectangularMapToVerticalCross(b));
    }

    Bitmap ComputeIrradianceEquirect(const Bitmap& inEquirect,
        int outWidth,
        int outHeight,
        int samples) const;

    void ConvolveDiffuse(const glm::vec3* data, int srcW, int srcH, int dstW, int dstH, glm::vec3* output, int numMonteCarloSamples) const;

    void SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) const;

    // NEW: Save as LDR (for skybox display)
    void SaveAsLDR(const Bitmap& image, const std::filesystem::path& outPath) const;

    // NEW: Save 6 cube faces as LDR images (e.g. PNG) instead of HDR
    void SaveFacesToDiskLDR(const Bitmap& cubeMap, const std::array<std::filesystem::path, 6>& facePaths, const std::string& prefix) const;

    std::vector<Bitmap> ComputePrefilteredCubemap(const Bitmap& inEquirect, int baseFaceSize, int numSamples) const;

private:
};