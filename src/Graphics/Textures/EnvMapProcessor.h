#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/vec4.hpp>

/**
 * Simple enum to specify how pixel data is stored
 */
enum eBitmapFormat
{
    eBitmapFormat_UnsignedByte,
    eBitmapFormat_Float,
};

/**
 * Simple enum to specify whether this Bitmap is 2D or a Cube (6 slices in depth).
 */
enum eBitmapType
{
    eBitmapType_2D,
    eBitmapType_Cube
};

/**
 * @struct Bitmap
 * @brief Represents an image (2D or 3D) with R/G/B/(A) channels in either float or 8-bit format.
 */
struct Bitmap
{
    Bitmap() = default;
    // 2D constructor
    Bitmap(int w, int h, int comp, eBitmapFormat fmt);
    // 3D constructor (e.g. for a cubemap with d_=6)
    Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt);

    // Width/Height/Depth (for cubemaps or array textures)
    int w_ = 0;
    int h_ = 0;
    int d_ = 1;     // if d_==6, often used for cubemaps
    int comp_ = 3;  // number of channels, e.g. 3=RGB, 4=RGBA
    eBitmapFormat fmt_ = eBitmapFormat_Float;
    eBitmapType   type_ = eBitmapType_2D;

    // Actual pixel data (size = w_ * h_ * d_ * comp_ * bytesPerComponent)
    std::vector<std::uint8_t> data_;

    // 2D getters/setters
    void        setPixel(int x, int y, const glm::vec4& c);
    glm::vec4   getPixel(int x, int y) const;

    // 3D getters/setters (useful if d_>1, e.g. cubemaps)
    void        setPixel3D(int x, int y, int z, const glm::vec4& c);
    glm::vec4   getPixel3D(int x, int y, int z) const;
};

/**
 * @class EnvMapProcessor
 * @brief Provides CPU-based methods to:
 *         - Load HDR equirectangular images,
 *         - Convert to vertical-cross/cubemap,
 *         - Save as HDR or PNG,
 *         - And compute a diffuse irradiance cubemap using Monte Carlo convolution.
 */
class EnvMapProcessor
{
public:
    /**
     * @brief Loads an HDR (floating-point) equirectangular image from disk.
     *        If the file is truly HDR, it uses float internally.
     *        If LDR, you could adapt, but here we assume float usage.
     */
    static Bitmap LoadEquirectangular(const std::string& filePath);

    /**
     * @brief Converts an equirectangular environment map into a vertical cross layout (3x4).
     */
    static Bitmap EquirectangularToVerticalCross(const Bitmap& equirectMap);

    /**
     * @brief Converts a vertical cross layout into a single Bitmap with d_=6 (cubemap).
     */
    static Bitmap VerticalCrossToCubemapFaces(const Bitmap& cross);

    /**
     * @brief Saves the given Bitmap as .hdr (if float) or .png (if 8-bit).
     *        For cubemap Bitmaps (d_=6), you must flatten or save slices yourself.
     */
    static void SaveAsHDR(const Bitmap& image, const std::string& outPath);
    static void SaveAsPNG(const Bitmap& image, const std::string& outPath);

    /**
     * @brief Computes a diffuse irradiance cubemap from an HDR equirect map,
     *        using a Monte Carlo convolution approach (Hammersley sampling).
     * @param equirectInput   Input environment (must be float, typically 2:1 aspect).
     * @param dstWidth        Width of the convolved equirect result (e.g., 64).
     * @param dstHeight       Height (e.g., 32).
     * @param numSamples      Number of samples in the integration (e.g., 512).
     * @return                A Bitmap with d_=6, containing the diffuse irradiance cubemap.
     */
    static Bitmap ComputeIrradianceCubemap(
        const Bitmap& equirectInput,
        int dstWidth,
        int dstHeight,
        int numSamples);

private:
    /**
     * @brief Samples an equirectangular map using spherical coords (theta, phi).
     *        theta in [-pi..pi], phi in [-pi/2..pi/2].
     */
    static glm::vec4 SampleEquirect(const Bitmap& equirect, float theta, float phi);

    // We'll define our private convolveDiffuse and Hammersley routines inside .cpp
};