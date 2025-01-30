#pragma once

#include <string>
#include "Bitmap.h"
#include <glm/vec4.hpp>

/**
 * @class EnvMapProcessor
 * @brief Provides CPU-based methods to handle HDR environment maps,
 *        convert them to vertical cross format, slice out cube faces,
 *        and save them as images.
 */
class EnvMapProcessor
{
public:
    /**
     * @brief Loads an HDR (or LDR) equirectangular image from disk into a Bitmap.
     *
     * For HDR, we store float data in the Bitmap. For LDR, we store 8-bit data.
     * Internally uses stb_image.
     */
    static Bitmap LoadEquirectangular(const std::string& filePath);

    /**
     * @brief Converts an equirectangular environment map into a vertical cross layout (2D).
     * @param equirectMap The loaded equirectangular map (typ. float).
     * @return A new Bitmap arranged as a 3x4 cross of faces.
     */
    static Bitmap EquirectangularToVerticalCross(const Bitmap& equirectMap);

    /**
     * @brief Extracts 6 faces from a vertical cross layout into either:
     *        - A single Bitmap with d_=6, or
     *        - (Optionally) returns 6 separate Bitmaps (not shown in this snippet).
     */
    static Bitmap VerticalCrossToCubemapFaces(const Bitmap& cross);

    
    /**
     * @brief Saves the given Bitmap as .hdr (if float) or .png (if 8-bit).
     */
    static void SaveAsHDR(const Bitmap& image, const std::string& outPath);
    static void SaveAsPNG(const Bitmap& image, const std::string& outPath);

private:
    /**
     * @brief Bilinear sampling from an equirectangular map using spherical coords.
     * @param equirect The input equirect Bitmap.
     * @param theta    Longitude in [-pi, pi].
     * @param phi      Latitude in [-pi/2, pi/2].
     */
    static glm::vec4 SampleEquirect(const Bitmap& equirect, float theta, float phi);
};