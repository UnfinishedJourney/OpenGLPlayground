#include "EnvMapProcessor.h"

// Include STB libraries
#include <stb_image.h>

#include <stb_image_write.h>

#include <stb_image_resize2.h> // Ensure this path is correct

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>   // for pi
//#include <glm/gtc/clamp.hpp>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

/***********************************************************************
 *                     Bitmap Implementation
 ***********************************************************************/
static int getBytesPerComponent(eBitmapFormat fmt)
{
    return (fmt == eBitmapFormat_Float) ? 4 : 1;
}

// 2D constructor
Bitmap::Bitmap(int w, int h, int comp, eBitmapFormat fmt)
    : w_(w), h_(h), d_(1), comp_(comp), fmt_(fmt)
{
    size_t size = static_cast<size_t>(w_) * h_ * comp_ * getBytesPerComponent(fmt_);
    data_.resize(size);
}

// 3D constructor
Bitmap::Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt)
    : w_(w), h_(h), d_(d), comp_(comp), fmt_(fmt)
{
    size_t size = static_cast<size_t>(w_) * h_ * d_ * comp_ * getBytesPerComponent(fmt_);
    data_.resize(size);
}

// Helpers for float or 8-bit
static void setPixelFloat(Bitmap& bmp, int x, int y, const glm::vec4& c)
{
    int ofs = (y * bmp.w_ + x) * bmp.comp_;
    float* ptr = reinterpret_cast<float*>(bmp.data_.data());
    for (int i = 0; i < bmp.comp_; i++)
        ptr[ofs + i] = c[i];
}

static glm::vec4 getPixelFloat(const Bitmap& bmp, int x, int y)
{
    int ofs = (y * bmp.w_ + x) * bmp.comp_;
    const float* ptr = reinterpret_cast<const float*>(bmp.data_.data());
    glm::vec4 c(0.0f);
    for (int i = 0; i < bmp.comp_; i++)
        c[i] = ptr[ofs + i];
    return c;
}

static void setPixelByte(Bitmap& bmp, int x, int y, const glm::vec4& c)
{
    int ofs = (y * bmp.w_ + x) * bmp.comp_;
    std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(bmp.data_.data());
    for (int i = 0; i < bmp.comp_; i++) {
        float val = glm::clamp(c[i], 0.0f, 1.0f) * 255.0f;
        ptr[ofs + i] = static_cast<std::uint8_t>(val + 0.5f);
    }
}

static glm::vec4 getPixelByte(const Bitmap& bmp, int x, int y)
{
    int ofs = (y * bmp.w_ + x) * bmp.comp_;
    const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(bmp.data_.data());
    glm::vec4 c(0.0f);
    for (int i = 0; i < bmp.comp_; i++)
        c[i] = static_cast<float>(ptr[ofs + i]) / 255.0f;
    return c;
}

// 2D setPixel
void Bitmap::setPixel(int x, int y, const glm::vec4& c)
{
    if (fmt_ == eBitmapFormat_Float) {
        setPixelFloat(*this, x, y, c);
    }
    else {
        setPixelByte(*this, x, y, c);
    }
}

// 2D getPixel
glm::vec4 Bitmap::getPixel(int x, int y) const
{
    if (fmt_ == eBitmapFormat_Float) {
        return getPixelFloat(*this, x, y);
    }
    else {
        return getPixelByte(*this, x, y);
    }
}

// 3D setPixel
void Bitmap::setPixel3D(int x, int y, int z, const glm::vec4& c)
{
    // Ensure z is within bounds
    if (z < 0 || z >= d_) {
        throw std::out_of_range("setPixel3D: z index out of range");
    }

    // Offset in slices
    int ofsSlice = z * (w_ * h_);
    int index = ofsSlice + (y * w_ + x);

    if (fmt_ == eBitmapFormat_Float) {
        float* ptr = reinterpret_cast<float*>(data_.data());
        int ofs = index * comp_;
        for (int i = 0; i < comp_; i++)
            ptr[ofs + i] = c[i];
    }
    else {
        std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(data_.data());
        int ofs = index * comp_;
        for (int i = 0; i < comp_; i++) {
            float val = glm::clamp(c[i], 0.0f, 1.0f);
            ptr[ofs + i] = static_cast<std::uint8_t>(val * 255.0f + 0.5f);
        }
    }
}

// 3D getPixel
glm::vec4 Bitmap::getPixel3D(int x, int y, int z) const
{
    // Ensure z is within bounds
    if (z < 0 || z >= d_) {
        throw std::out_of_range("getPixel3D: z index out of range");
    }

    int ofsSlice = z * (w_ * h_);
    int index = ofsSlice + (y * w_ + x);

    glm::vec4 ret(0.0f);
    if (fmt_ == eBitmapFormat_Float) {
        const float* ptr = reinterpret_cast<const float*>(data_.data());
        int ofs = index * comp_;
        for (int i = 0; i < comp_; i++)
            ret[i] = ptr[ofs + i];
    }
    else {
        const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(data_.data());
        int ofs = index * comp_;
        for (int i = 0; i < comp_; i++)
            ret[i] = static_cast<float>(ptr[ofs + i]) / 255.0f;
    }
    return ret;
}

/***********************************************************************
 *                     EnvMapProcessor Implementation
 ***********************************************************************/

 // Hammersley Sequence Helpers
static float radicalInverse_VdC(uint32_t bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return static_cast<float>(bits) * 2.3283064365386963e-10f; // 1/(1<<32)
}

static glm::vec2 hammersley2d(uint32_t i, uint32_t N)
{
    return glm::vec2(static_cast<float>(i) / static_cast<float>(N), radicalInverse_VdC(i));
}

/**
 * @brief Performs Monte Carlo convolution to compute the diffuse irradiance.
 * @param data       Input environment map data as an array of glm::vec3.
 * @param srcW       Source width.
 * @param srcH       Source height.
 * @param dstW       Destination width.
 * @param dstH       Destination height.
 * @param output     Output array to store the convolved results.
 * @param numSamples Number of samples for Monte Carlo integration.
 */
static void convolveDiffuse(
    const glm::vec3* data,
    int srcW, int srcH,
    int dstW, int dstH,
    glm::vec3* output,
    int numSamples)
{
    // Check equirect aspect
    if (srcW != 2 * srcH) {
        std::cerr << "convolveDiffuse: input must be 2:1 equirect" << std::endl;
        return;
    }

    // 1) Downsample with stbir_resize_float_linear
    std::vector<glm::vec3> tmp(dstW * dstH);
    float* tmp_ptr = reinterpret_cast<float*>(tmp.data());

    float* resize_result = stbir_resize_float_linear(
        reinterpret_cast<const float*>(data), // input
        srcW, srcH, 0,                        // input dimensions and stride (0 for default)
        tmp_ptr, dstW, dstH, 0,               // output buffer and dimensions
        STBIR_RGB                             // pixel layout
    );

    if (resize_result == nullptr) {
        throw std::runtime_error("stbir_resize_float_linear failed!");
    }

    // We'll treat "tmp" as the working environment
    const glm::vec3* scratch = tmp.data();

    // 2) For each pixel (y,x) in [0..dstH-1, 0..dstW-1],
    //    perform hemisphere integral via Hammersley sampling
    for (int y = 0; y < dstH; y++)
    {
        // theta1 in [0..pi]
        float theta1 = static_cast<float>(y) / static_cast<float>(dstH) * glm::pi<float>();
        for (int x = 0; x < dstW; x++)
        {
            // phi1 in [0..2pi]
            float phi1 = static_cast<float>(x) / static_cast<float>(dstW) * (2.f * glm::pi<float>());
            glm::vec3 V1(
                std::sin(theta1) * std::cos(phi1),
                std::sin(theta1) * std::sin(phi1),
                std::cos(theta1)
            );

            glm::vec3 color(0.f);
            float weight = 0.f;

            for (int i = 0; i < numSamples; i++)
            {
                glm::vec2 h = hammersley2d(static_cast<uint32_t>(i), static_cast<uint32_t>(numSamples));
                // Map Hammersley sample to [0..dstW)x[0..dstH)
                int x1 = static_cast<int>(std::floor(h.x * dstW));
                int y1 = static_cast<int>(std::floor(h.y * dstH));

                // Clamp to valid range
                x1 = glm::clamp(x1, 0, dstW - 1);
                y1 = glm::clamp(y1, 0, dstH - 1);

                float theta2 = static_cast<float>(y1) / static_cast<float>(dstH) * glm::pi<float>();
                float phi2 = static_cast<float>(x1) / static_cast<float>(dstW) * (2.f * glm::pi<float>());
                glm::vec3 V2(
                    std::sin(theta2) * std::cos(phi2),
                    std::sin(theta2) * std::sin(phi2),
                    std::cos(theta2)
                );

                float D = glm::max(0.f, glm::dot(V1, V2));
                if (D > 0.01f)
                {
                    color += scratch[y1 * dstW + x1] * D;
                    weight += D;
                }
            }

            if (weight > 1e-5f)
                color /= weight;
            output[y * dstW + x] = color;
        }
    }
}
//
// 1. LoadEquirectangular
//
Bitmap EnvMapProcessor::LoadEquirectangular(const std::string& filePath)
{
    // We assume HDR => float
    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    float* data = stbi_loadf(filePath.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::ostringstream oss;
        oss << "EnvMapProcessor::LoadEquirectangular: Failed to open " << filePath;
        throw std::runtime_error(oss.str());
    }

    if (channels < 3) {
        stbi_image_free(data);
        throw std::runtime_error("LoadEquirectangular: image has fewer than 3 channels!");
    }

    // We'll store 3 or 4 comps
    int desiredComp = (channels >= 4) ? 4 : 3;

    // Make a float-based Bitmap
    Bitmap result(width, height, desiredComp, eBitmapFormat_Float);

    size_t totalPixels = static_cast<size_t>(width) * height;
    float* dst = reinterpret_cast<float*>(result.data_.data());

    for (size_t i = 0; i < totalPixels; i++)
    {
        float r = data[i * channels + 0];
        float g = (channels > 1) ? data[i * channels + 1] : 0.f;
        float b = (channels > 2) ? data[i * channels + 2] : 0.f;
        float a = (desiredComp == 4 && channels >= 4)
            ? data[i * channels + 3] : 1.f;

        dst[i * desiredComp + 0] = r;
        if (desiredComp >= 2) dst[i * desiredComp + 1] = g;
        if (desiredComp >= 3) dst[i * desiredComp + 2] = b;
        if (desiredComp >= 4) dst[i * desiredComp + 3] = a;
    }

    stbi_image_free(data);
    return result;
}

//
// Helper: SampleEquirect
//
glm::vec4 EnvMapProcessor::SampleEquirect(const Bitmap& equirect, float theta, float phi)
{
    int w = equirect.w_;
    int h = equirect.h_;

    // Range of theta: [-pi..pi], map to [0..1]
    float u = (theta + glm::pi<float>()) / (2.f * glm::pi<float>());
    // Range of phi: [-pi/2..pi/2], map to [0..1]
    float v = 0.5f - (phi / glm::pi<float>());

    // Wrap u
    if (u < 0.f) u += 1.f;
    if (u > 1.f) u -= 1.f;
    // Clamp v
    v = glm::clamp(v, 0.f, 1.f);

    // Convert to pixel coords
    float x = u * static_cast<float>(w - 1);
    float y = v * static_cast<float>(h - 1);

    int x1 = static_cast<int>(std::floor(x));
    int y1 = static_cast<int>(std::floor(y));
    int x2 = std::min(x1 + 1, w - 1);
    int y2 = std::min(y1 + 1, h - 1);

    float fx = x - static_cast<float>(x1);
    float fy = y - static_cast<float>(y1);

    glm::vec4 A = equirect.getPixel(x1, y1);
    glm::vec4 B = equirect.getPixel(x2, y1);
    glm::vec4 C = equirect.getPixel(x1, y2);
    glm::vec4 D = equirect.getPixel(x2, y2);

    glm::vec4 col = A * (1.f - fx) * (1.f - fy)
        + B * (fx) * (1.f - fy)
        + C * (1.f - fx) * fy
        + D * (fx * fy);
    return col;
}

//
// 2. EquirectangularToVerticalCross
//
static glm::vec3 FaceCoordsToXYZ(int i, int j, int faceID, int faceSize)
{
    // Map i,j in [0..faceSize-1] -> [ -1..1 ]
    float A = 2.f * ((static_cast<float>(i) / static_cast<float>(faceSize)) - 0.5f);
    float B = 2.f * ((static_cast<float>(j) / static_cast<float>(faceSize)) - 0.5f);

    // faceID -> direction
    // example layout: +X, -X, +Y, -Y, +Z, -Z
    switch (faceID)
    {
    case 0: // +X
        return glm::normalize(glm::vec3(1.f, -B, -A));
    case 1: // -X
        return glm::normalize(glm::vec3(-1.f, -B, A));
    case 2: // +Y
        return glm::normalize(glm::vec3(A, 1.f, B));
    case 3: // -Y
        return glm::normalize(glm::vec3(A, -1.f, -B));
    case 4: // +Z
        return glm::normalize(glm::vec3(A, -B, 1.f));
    case 5: // -Z
        return glm::normalize(glm::vec3(-A, -B, -1.f));
    default:
        return glm::vec3(0.f);
    }
}

Bitmap EnvMapProcessor::EquirectangularToVerticalCross(const Bitmap& equirectMap)
{
    int eqW = equirectMap.w_;
    int eqH = equirectMap.h_;
    int eqComp = equirectMap.comp_;

    // faceSize typically eqW / 4
    int faceSize = eqW / 4;
    int crossW = faceSize * 3;
    int crossH = faceSize * 4;

    Bitmap cross(crossW, crossH, eqComp, equirectMap.fmt_);

    // Offsets in a typical vertical cross layout
    // +X  = face 0 => (2*fs, 1*fs)
    // -X  = face 1 => (0,    1*fs)
    // +Y  = face 2 => (1*fs, 0)
    // -Y  = face 3 => (1*fs, 2*fs)
    // +Z  = face 4 => (1*fs, 1*fs)
    // -Z  = face 5 => (1*fs, 3*fs)
    glm::ivec2 offsets[6] = {
        {2 * faceSize, 1 * faceSize},
        {0,          1 * faceSize},
        {1 * faceSize, 0        },
        {1 * faceSize, 2 * faceSize},
        {1 * faceSize, 1 * faceSize},
        {1 * faceSize, 3 * faceSize}
    };

    // Fill in each face
    for (int faceID = 0; faceID < 6; faceID++)
    {
        glm::ivec2 off = offsets[faceID];
        for (int j = 0; j < faceSize; j++)
        {
            for (int i = 0; i < faceSize; i++)
            {
                glm::vec3 dir = FaceCoordsToXYZ(i, j, faceID, faceSize);

                // Spherical angles for dir
                float theta = std::atan2(dir.z, dir.x);    // [-pi..pi]
                float r = std::sqrt(dir.x * dir.x + dir.z * dir.z);
                float phi = std::atan2(dir.y, r);        // [-pi/2..pi/2]

                glm::vec4 color = SampleEquirect(equirectMap, theta, phi);
                cross.setPixel(off.x + i, off.y + j, color);
            }
        }
    }

    return cross;
}

//
// 3. VerticalCrossToCubemapFaces
//
Bitmap EnvMapProcessor::VerticalCrossToCubemapFaces(const Bitmap& cross)
{
    int crossW = cross.w_;
    int crossH = cross.h_;
    int comp = cross.comp_;
    eBitmapFormat fmt = cross.fmt_;

    // faceW = crossW / 3, faceH = crossH / 4
    int faceW = crossW / 3;
    int faceH = crossH / 4;

    Bitmap cubemap(faceW, faceH, 6, comp, fmt);
    cubemap.type_ = eBitmapType_Cube;

    // Offsets as defined in EquirectangularToVerticalCross
    glm::ivec2 offsets[6] = {
        {2 * faceW, 1 * faceH}, // +X
        {0,       1 * faceH},   // -X
        {faceW,   0       },    // +Y
        {faceW,   2 * faceH},   // -Y
        {faceW,   faceH  },     // +Z
        {faceW,   3 * faceH}    // -Z
    };

    for (int faceID = 0; faceID < 6; faceID++)
    {
        glm::ivec2 off = offsets[faceID];
        for (int j = 0; j < faceH; j++)
        {
            for (int i = 0; i < faceW; i++)
            {
                glm::vec4 c = cross.getPixel(off.x + i, off.y + j);
                cubemap.setPixel3D(i, j, faceID, c);
            }
        }
    }

    return cubemap;
}

//
// 4. SaveAsHDR / SaveAsPNG
//
void EnvMapProcessor::SaveAsHDR(const Bitmap& image, const std::string& outPath)
{
    // Must be float
    if (image.fmt_ != eBitmapFormat_Float) {
        throw std::runtime_error("SaveAsHDR: image must be float format!");
    }

    // stbi_write_hdr can handle comp=3 or comp=4
    int w = image.w_;
    int h = image.h_;
    int c = image.comp_;
    if (c < 3 || c > 4) {
        throw std::runtime_error("SaveAsHDR: only 3 or 4 channels supported!");
    }
    if (image.d_ != 1) {
        // stbi can't handle 3D/cubemap directly; you'd flatten or save faces.
        throw std::runtime_error("SaveAsHDR: cannot save 3D/cubemap directly, flatten first!");
    }

    const float* ptr = reinterpret_cast<const float*>(image.data_.data());
    if (!stbi_write_hdr(outPath.c_str(), w, h, c, ptr)) {
        throw std::runtime_error("stbi_write_hdr failed!");
    }
}

void EnvMapProcessor::SaveAsPNG(const Bitmap& image, const std::string& outPath)
{
    // Must be 8-bit
    if (image.fmt_ != eBitmapFormat_UnsignedByte) {
        throw std::runtime_error("SaveAsPNG: image must be 8-bit format!");
    }
    int w = image.w_;
    int h = image.h_;
    int c = image.comp_;
    if (c < 1 || c > 4) {
        throw std::runtime_error("SaveAsPNG: channels must be 1..4");
    }

    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(image.data_.data());
    // stride in bytes:
    int stride = w * c;
    if (!stbi_write_png(outPath.c_str(), w, h, c, ptr, stride)) {
        throw std::runtime_error("stbi_write_png failed!");
    }
}

//
// 5. ComputeIrradianceCubemap
//
Bitmap EnvMapProcessor::ComputeIrradianceCubemap(
    const Bitmap& equirectInput,
    int dstWidth,
    int dstHeight,
    int numSamples)
{
    // Must be float
    if (equirectInput.fmt_ != eBitmapFormat_Float) {
        throw std::runtime_error("ComputeIrradianceCubemap: input must be float format!");
    }
    // Must have at least 3 channels
    if (equirectInput.comp_ < 3) {
        throw std::runtime_error("ComputeIrradianceCubemap: need at least RGB channels!");
    }
    // Must be 2:1
    if (equirectInput.w_ != 2 * equirectInput.h_) {
        throw std::runtime_error("ComputeIrradianceCubemap: equirect must be 2:1 aspect!");
    }

    // 1) Copy from 'equirectInput' into a float3 array
    int srcW = equirectInput.w_;
    int srcH = equirectInput.h_;
    int srcComp = equirectInput.comp_;
    const float* srcPtr = reinterpret_cast<const float*>(equirectInput.data_.data());

    std::vector<glm::vec3> inData(srcW * srcH);
    for (int i = 0; i < srcW * srcH; i++) {
        float r = srcPtr[i * srcComp + 0];
        float g = (srcComp > 1) ? srcPtr[i * srcComp + 1] : 0.f;
        float b = (srcComp > 2) ? srcPtr[i * srcComp + 2] : 0.f;
        inData[i] = glm::vec3(r, g, b);
    }

    // 2) Prepare array for output convolved equirect
    std::vector<glm::vec3> outData(dstWidth * dstHeight);

    // 3) Call convolveDiffuse
    convolveDiffuse(inData.data(), srcW, srcH,
        dstWidth, dstHeight,
        outData.data(), numSamples);

    // 4) Wrap outData as a Bitmap
    Bitmap convolvedEquirect(dstWidth, dstHeight, srcComp, equirectInput.fmt_);
    float* dstPtr = reinterpret_cast<float*>(convolvedEquirect.data_.data());
    for (int i = 0; i < dstWidth * dstHeight; i++) {
        glm::vec3 c = outData[i];
        dstPtr[i * srcComp + 0] = c.x;
        if (srcComp >= 2) dstPtr[i * srcComp + 1] = c.y;
        if (srcComp >= 3) dstPtr[i * srcComp + 2] = c.z;
        if (srcComp >= 4) dstPtr[i * srcComp + 3] = 1.f; // alpha=1
    }

    // 5) Convert equirect -> cross
    Bitmap cross = EquirectangularToVerticalCross(convolvedEquirect);

    // 6) Convert cross -> cubemap
    Bitmap cube = VerticalCrossToCubemapFaces(cross);

    // done
    return cube;
}