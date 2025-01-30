#include "EnvMapProcessor.h"

#include <stb_image.h>

#include <stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <sstream>

/**********************************************
 *         1. LoadEquirectangular
 **********************************************/
Bitmap EnvMapProcessor::LoadEquirectangular(const std::string& filePath)
{
    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    // Here, we expect float data for HDR. If the file is LDR, you'd adapt or detect
    float* data = stbi_loadf(filePath.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::ostringstream oss;
        oss << "EnvMapProcessor::LoadEquirectangular: Failed to open " << filePath;
        throw std::runtime_error(oss.str());
    }

    if (channels < 3) {
        stbi_image_free(data);
        throw std::runtime_error("Equirectangular image has less than 3 channels!");
    }

    // Typically we want 3 or 4 components. If channels >= 4, we store 4; else 3.
    int desiredComponents = (channels >= 4) ? 4 : 3;

    // Allocate a float-based Bitmap
    Bitmap result(width, height, desiredComponents, eBitmapFormat::eBitmapFormat_Float);

    // Copy from STB buffer into our Bitmap
    size_t totalPixels = size_t(width) * size_t(height);
    float* dst = reinterpret_cast<float*>(result.data_.data());

    for (size_t i = 0; i < totalPixels; ++i)
    {
        float r = data[i * channels + 0];
        float g = (channels > 1) ? data[i * channels + 1] : 0.0f;
        float b = (channels > 2) ? data[i * channels + 2] : 0.0f;
        float a = (desiredComponents == 4 && channels >= 4)
            ? data[i * channels + 3] : 1.0f;

        // Write to destination
        dst[i * desiredComponents + 0] = r;
        if (desiredComponents >= 2) dst[i * desiredComponents + 1] = g;
        if (desiredComponents >= 3) dst[i * desiredComponents + 2] = b;
        if (desiredComponents == 4) dst[i * desiredComponents + 3] = a;
    }

    stbi_image_free(data);
    return result;
}

/**********************************************
 *       2. EquirectangularToVerticalCross
 **********************************************/
static glm::vec3 FaceCoordsToXYZ(int i, int j, int faceID, int faceSize)
{
    // This example uses the typical approach of mapping each face to a [-1,1] range 
    float A = 2.0f * (float(i) / float(faceSize)) - 1.0f;
    float B = 2.0f * (float(j) / float(faceSize)) - 1.0f;

    // faceID -> direction
    switch (faceID)
    {
    case 0: // +X
        return glm::normalize(glm::vec3(1.0f, B, -A));
    case 1: // -X
        return glm::normalize(glm::vec3(-1.0f, B, A));
    case 2: // +Y
        return glm::normalize(glm::vec3(A, 1.0f, -B));
    case 3: // -Y
        return glm::normalize(glm::vec3(A, -1.0f, B));
    case 4: // +Z
        return glm::normalize(glm::vec3(A, B, 1.0f));
    case 5: // -Z
        return glm::normalize(glm::vec3(-A, B, -1.0f));
    }
    return glm::vec3(0.0f);
}

Bitmap EnvMapProcessor::EquirectangularToVerticalCross(const Bitmap& equirectMap)
{
    int eqW = equirectMap.w_;
    int eqH = equirectMap.h_;
    int eqComp = equirectMap.comp_;

    // Face size is typically eqW / 4
    int faceSize = eqW / 4;
    // The cross is 3 faces wide, 4 faces tall
    int crossW = faceSize * 3;
    int crossH = faceSize * 4;

    Bitmap cross(crossW, crossH, eqComp, equirectMap.fmt_);

    // Offsets for the 6 faces in a standard vertical cross layout
    // face=0 => +X at (2*faceSize, 1*faceSize)
    // face=1 => -X at (0,         1*faceSize)
    // face=2 => +Y at (1*faceSize,0)
    // face=3 => -Y at (1*faceSize,2*faceSize)
    // face=4 => +Z at (1*faceSize,1*faceSize)
    // face=5 => -Z at (1*faceSize,3*faceSize)
    // This is just one possible arrangement
    const glm::ivec2 offsets[6] = {
        { 2 * faceSize, 1 * faceSize }, // +X
        { 0,          1 * faceSize }, // -X
        { 1 * faceSize, 0          }, // +Y
        { 1 * faceSize, 2 * faceSize }, // -Y
        { 1 * faceSize, 1 * faceSize }, // +Z
        { 1 * faceSize, 3 * faceSize }  // -Z
    };

    // For each face, fill in the cross
    for (int faceID = 0; faceID < 6; faceID++)
    {
        glm::ivec2 off = offsets[faceID];
        for (int j = 0; j < faceSize; j++)
        {
            for (int i = 0; i < faceSize; i++)
            {
                glm::vec3 dir = FaceCoordsToXYZ(i, j, faceID, faceSize);

                // Convert direction to spherical angles
                //  theta in [-pi, pi], phi in [-pi/2, pi/2]
                float theta = std::atan2(dir.z, dir.x); // range -pi..pi
                float r = std::sqrt(dir.x * dir.x + dir.z * dir.z);
                float phi = std::atan2(dir.y, r);      // range -pi/2..pi/2

                glm::vec4 color = SampleEquirect(equirectMap, theta, phi);
                cross.setPixel(off.x + i, off.y + j, color);
            }
        }
    }

    return cross;
}

/**********************************************
 *        SampleEquirect
 **********************************************/
 // We interpret equirect as: 
 //   u in [0..1] ~ theta in [-pi..pi]
 //   v in [0..1] ~ phi in [-pi/2..pi/2]
glm::vec4 EnvMapProcessor::SampleEquirect(const Bitmap& equirect, float theta, float phi)
{
    int w = equirect.w_;
    int h = equirect.h_;

    // Convert angles to [0..1]
    float u = (theta + glm::pi<float>()) / (2.0f * glm::pi<float>());
    float v = (glm::pi<float>() * 0.5f - phi) / glm::pi<float>();

    // wrap u, clamp v
    if (u < 0.0f) u += 1.0f;
    if (u > 1.0f) u -= 1.0f;
    v = std::clamp(v, 0.0f, 1.0f);

    // Convert to pixel coords
    float x = u * (w - 1);
    float y = v * (h - 1);

    int x1 = int(std::floor(x));
    int y1 = int(std::floor(y));
    int x2 = std::min(x1 + 1, w - 1);
    int y2 = std::min(y1 + 1, h - 1);

    float fracX = x - float(x1);
    float fracY = y - float(y1);

    glm::vec4 A = equirect.getPixel(x1, y1);
    glm::vec4 B = equirect.getPixel(x2, y1);
    glm::vec4 C = equirect.getPixel(x1, y2);
    glm::vec4 D = equirect.getPixel(x2, y2);

    // bilinear
    glm::vec4 color = A * (1 - fracX) * (1 - fracY)
        + B * (fracX) * (1 - fracY)
        + C * (1 - fracX) * (fracY)
        +D * (fracX) * (fracY);
    return color;
}

/**********************************************
 * 3. VerticalCrossToCubemapFaces
 **********************************************/
Bitmap EnvMapProcessor::VerticalCrossToCubemapFaces(const Bitmap& cross)
{
    int crossW = cross.w_;
    int crossH = cross.h_;
    int comp = cross.comp_;
    eBitmapFormat fmt = cross.fmt_;

    // Each face
    int faceW = crossW / 3;
    int faceH = crossH / 4;

    // We'll store them in one Bitmap with d_=6
    Bitmap cube(faceW, faceH, 6, comp, fmt);
    cube.type_ = eBitmapType::eBitmapType_Cube;

    // same offsets as in EquirectangularToVerticalCross
    const glm::ivec2 offsets[6] = {
        { 2 * faceW, 1 * faceH }, // +X
        { 0,        1 * faceH }, // -X
        { 1 * faceW,  0      }, // +Y
        { 1 * faceW,  2 * faceH }, // -Y
        { 1 * faceW,  1 * faceH }, // +Z
        { 1 * faceW,  3 * faceH }  // -Z
    };

    for (int faceID = 0; faceID < 6; faceID++)
    {
        glm::ivec2 off = offsets[faceID];
        for (int j = 0; j < faceH; j++)
        {
            for (int i = 0; i < faceW; i++)
            {
                glm::vec4 color = cross.getPixel(off.x + i, off.y + j);
                // Put into z=faceID
                cube.setPixel3D(i, j, faceID, color);
            }
        }
    }

    return cube;
}

/**********************************************
 * 4. SaveAsHDR / SaveAsPNG
 **********************************************/
void EnvMapProcessor::SaveAsHDR(const Bitmap& image, const std::string& outPath)
{
    if (image.fmt_ != eBitmapFormat::eBitmapFormat_Float)
        throw std::runtime_error("SaveAsHDR: Bitmap must be float format!");

    int width = image.w_;
    int height = image.h_;
    int comp = image.comp_;

    if (comp < 3) {
        throw std::runtime_error("SaveAsHDR: Must have at least 3 channels (RGB).");
    }

    const float* data = reinterpret_cast<const float*>(image.data_.data());

    // stbi_write_hdr supports comp=3 or comp=4
    if (comp > 4) {
        throw std::runtime_error("SaveAsHDR: More than 4 channels not supported by stbi.");
    }

    // If comp=3 or comp=4
    int res = stbi_write_hdr(outPath.c_str(), width, height, comp, data);
    if (!res) {
        throw std::runtime_error("stbi_write_hdr failed!");
    }
}

void EnvMapProcessor::SaveAsPNG(const Bitmap& image, const std::string& outPath)
{
    if (image.fmt_ != eBitmapFormat::eBitmapFormat_UnsignedByte) {
        throw std::runtime_error("SaveAsPNG: Bitmap must be 8-bit format!");
    }

    int width = image.w_;
    int height = image.h_;
    int comp = image.comp_;

    // stbi_write_png can handle comp in [1..4]
    if (comp < 1 || comp > 4) {
        throw std::runtime_error("SaveAsPNG: invalid number of channels for PNG.");
    }

    const uint8_t* data = image.data_.data();
    // stride in bytes per row:
    int stride_in_bytes = width * comp;

    int res = stbi_write_png(outPath.c_str(), width, height, comp, data, stride_in_bytes);
    if (!res) {
        throw std::runtime_error("stbi_write_png failed!");
    }
}