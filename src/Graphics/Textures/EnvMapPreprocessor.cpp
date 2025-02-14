#include "EnvMapPreprocessor.h"
#include "Bitmap.h"
#include "Utilities/Logger.h"
#include "TextureUtils.h"  
#include <stb_image_write.h>
#include <stb_image_resize2.h>
#include <cmath>
#include <stdexcept>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

namespace graphics {

    // Some math constants
    namespace MathConstants {
        constexpr float PI = 3.14159265359f;
        constexpr float TWO_PI = 6.28318530718f;
    }

    // --- Internal utility functions in an anonymous namespace ---
    namespace {
        template <typename T>
        T clamp(T v, T lo, T hi) {
            return (v < lo) ? lo : ((v > hi) ? hi : v);
        }

        glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize) {
            float A = 2.0f * static_cast<float>(i) / faceSize;
            float B = 2.0f * static_cast<float>(j) / faceSize;
            switch (faceID) {
            case 0: return glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
            case 1: return glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
            case 2: return glm::vec3(1.0f, A - 1.0f, 1.0f - B);
            case 3: return glm::vec3(1.0f - A, 1.0f, 1.0f - B);
            case 4: return glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
            case 5: return glm::vec3(1.0f - B, A - 1.0f, -1.0f);
            default: return glm::vec3(0.0f);
            }
        }

        float radicalInverse_VdC(uint32_t bits) {
            bits = (bits << 16u) | (bits >> 16u);
            bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
            bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
            bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
            bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
            return float(bits) * 2.3283064365386963e-10f;
        }

        glm::vec2 hammersley2d(uint32_t i, uint32_t N) {
            return glm::vec2(static_cast<float>(i) / static_cast<float>(N), radicalInverse_VdC(i));
        }

        glm::vec3 ImportanceSampleGGX(const glm::vec2 Xi, const glm::vec3& N, float roughness) {
            float a = roughness * roughness;
            float phi = 2.0f * MathConstants::PI * Xi.x;
            float cosTheta = std::sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
            float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
            glm::vec3 H(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

            glm::vec3 up = (std::fabs(N.z) < 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
            glm::vec3 tangentX = glm::normalize(glm::cross(up, N));
            glm::vec3 tangentY = glm::cross(N, tangentX);

            return glm::normalize(tangentX * H.x + tangentY * H.y + N * H.z);
        }

        // Placeholder
        glm::vec3 SampleEquirectangular(const Bitmap& /*envMap*/, const glm::vec3& /*dir*/) {
            return glm::vec3(0.0f);
        }
    } // end anonymous namespace

    // --- EnvMapPreprocessor public methods ---

    Bitmap EnvMapPreprocessor::LoadTexture(const std::filesystem::path& texturePath) const {
        Bitmap tex;
        // For example, force 4 channels and do not flip Y.
        tex.LoadFromFile(texturePath.string(), /*flipY=*/false, /*force4Ch=*/true);
        return tex;
    }

    Bitmap EnvMapPreprocessor::ConvertEquirectangularMapToVerticalCross(const Bitmap& b) const {
        if (b.type() != BitmapType::TwoD) {
            return Bitmap(); // Return empty Bitmap if not 2D.
        }
        const int faceSize = b.width() / 4;
        const int outW = faceSize * 3;
        const int outH = faceSize * 4;
        Bitmap result(outW, outH, b.components(), b.format());

        // Offsets for the six faces in the vertical cross layout.
        const std::array<glm::ivec2, 6> offsets = {
            glm::ivec2(faceSize, faceSize * 3),
            glm::ivec2(0,         faceSize),
            glm::ivec2(faceSize,  faceSize),
            glm::ivec2(faceSize * 2, faceSize),
            glm::ivec2(faceSize,  0),
            glm::ivec2(faceSize,  faceSize * 2)
        };

        const int clampW = b.width() - 1;
        const int clampH = b.height() - 1;

        for (int face = 0; face < 6; ++face) {
            for (int i = 0; i < faceSize; ++i) {
                for (int j = 0; j < faceSize; ++j) {
                    glm::vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
                    float theta = std::atan2(P.y, P.x);
                    float phi = std::atan2(P.z, std::hypot(P.x, P.y));
                    float Uf = 2.0f * faceSize * (theta + MathConstants::PI) / MathConstants::PI;
                    float Vf = 2.0f * faceSize * ((MathConstants::PI / 2.0f - phi) / MathConstants::PI);

                    const int U1 = clamp(static_cast<int>(std::floor(Uf)), 0, clampW);
                    const int V1 = clamp(static_cast<int>(std::floor(Vf)), 0, clampH);
                    const int U2 = clamp(U1 + 1, 0, clampW);
                    const int V2 = clamp(V1 + 1, 0, clampH);

                    const float s = Uf - U1;
                    const float t = Vf - V1;

                    const glm::vec4 A = b.getPixel(U1, V1);
                    const glm::vec4 B = b.getPixel(U2, V1);
                    const glm::vec4 C = b.getPixel(U1, V2);
                    const glm::vec4 D = b.getPixel(U2, V2);

                    glm::vec4 color = A * (1 - s) * (1 - t) +
                        B * s * (1 - t) +
                        C * (1 - s) * t +
                        D * s * t;
                    result.setPixel(i + offsets[face].x, j + offsets[face].y, color);
                }
            }
        }
        return result;
    }

    Bitmap EnvMapPreprocessor::ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) const {
        const int faceW = b.width() / 3;
        const int faceH = b.height() / 4;
        // Create a cubemap Bitmap with 6 faces.
        Bitmap cube(faceW, faceH, 6, b.components(), b.format());

        const int pixelSize = b.components() * Bitmap::getBytesPerComponent(b.format());
        const int faceSizeBytes = faceW * faceH * pixelSize;
        const uint8_t* src = b.data().data();
        uint8_t* dst = cube.data().data();

        for (int face = 0; face < 6; ++face) {
            for (int j = 0; j < faceH; ++j) {
                for (int i = 0; i < faceW; ++i) {
                    int x = 0, y = 0;
                    switch (face) {
                    case 0: x = i;               y = faceH + j;        break;
                    case 1: x = 2 * faceW + i;     y = faceH + j;        break;
                    case 2: x = 2 * faceW - (i + 1); y = faceH - (j + 1);   break;
                    case 3: x = 2 * faceW - (i + 1); y = 3 * faceH - (j + 1); break;
                    case 4: x = 2 * faceW - (i + 1); y = b.height() - (j + 1); break;
                    case 5: x = faceW + i;         y = faceH + j;        break;
                    }
                    std::memcpy(dst, src + (y * b.width() + x) * pixelSize, pixelSize);
                    dst += pixelSize;
                }
            }
        }
        return cube;
    }

    Bitmap EnvMapPreprocessor::ComputeIrradianceEquirect(const Bitmap& inEquirect,
        int outW, int outH,
        int samples) const {
        if (inEquirect.format() != BitmapFormat::Float) {
            throw std::runtime_error("ComputeIrradianceEquirect: Input must be float format!");
        }
        if (inEquirect.data().empty()) {
            throw std::runtime_error("ComputeIrradianceEquirect: Input image is empty!");
        }
        const int srcW = inEquirect.width();
        const int srcH = inEquirect.height();
        if (srcW != 2 * srcH) {
            throw std::runtime_error("ComputeIrradianceEquirect: Equirectangular image must have width=2*height!");
        }

        // Create an output Bitmap with 3 channels.
        Bitmap outBmp(outW, outH, 3, BitmapFormat::Float);
        const glm::vec3* srcData = reinterpret_cast<const glm::vec3*>(inEquirect.data().data());
        glm::vec3* dstData = reinterpret_cast<glm::vec3*>(outBmp.data().data());

        for (int y = 0; y < outH; ++y) {
            const float theta1 = static_cast<float>(y) / outH * MathConstants::PI;
            for (int x = 0; x < outW; ++x) {
                const float phi1 = static_cast<float>(x) / outW * MathConstants::TWO_PI;
                glm::vec3 V1(std::sin(theta1) * std::cos(phi1),
                    std::sin(theta1) * std::sin(phi1),
                    std::cos(theta1));
                glm::vec3 color(0.0f);
                float weight = 0.0f;
                for (int i = 0; i < samples; ++i) {
                    glm::vec2 Xi = hammersley2d(i, samples);
                    const int x1 = static_cast<int>(std::floor(Xi.x * srcW));
                    const int y1 = static_cast<int>(std::floor(Xi.y * srcH));
                    const float theta2 = static_cast<float>(y1) / srcH * MathConstants::PI;
                    const float phi2 = static_cast<float>(x1) / srcW * MathConstants::TWO_PI;
                    glm::vec3 V2(std::sin(theta2) * std::cos(phi2),
                        std::sin(theta2) * std::sin(phi2),
                        std::cos(theta2));
                    const float D = std::max(glm::dot(V1, V2), 0.0f);
                    if (D > 0.01f) {
                        color += srcData[y1 * srcW + x1] * D;
                        weight += D;
                    }
                }
                dstData[y * outW + x] = (weight > 0.0f) ? (color / weight) : color;
            }
        }
        return outBmp;
    }

    std::vector<Bitmap> EnvMapPreprocessor::ComputePrefilteredCubemap(const Bitmap& inEquirect,
        int baseFaceSize,
        int numSamples) const {
        const int mipLevels = static_cast<int>(std::floor(std::log2(static_cast<float>(baseFaceSize)))) + 1;
        std::vector<Bitmap> mipMaps;
        mipMaps.reserve(mipLevels);

        for (int mip = 0; mip < mipLevels; ++mip) {
            const int faceSize = baseFaceSize >> mip;
            Bitmap mipCube(faceSize, faceSize, 6, inEquirect.components(), inEquirect.format());
            mipCube.SetType(BitmapType::Cube);

            const float roughness = (mipLevels > 1) ? static_cast<float>(mip) / (mipLevels - 1) : 0.0f;
            for (int face = 0; face < 6; ++face) {
                for (int y = 0; y < faceSize; ++y) {
                    for (int x = 0; x < faceSize; ++x) {
                        const float u = 2.0f * (x + 0.5f) / faceSize - 1.0f;
                        const float v = 2.0f * (y + 0.5f) / faceSize - 1.0f;
                        glm::vec3 R;
                        switch (face) {
                        case 0: R = glm::vec3(1.0f, -v, -u); break;
                        case 1: R = glm::vec3(-1.0f, -v, u); break;
                        case 2: R = glm::vec3(u, 1.0f, v); break;
                        case 3: R = glm::vec3(u, -1.0f, -v); break;
                        case 4: R = glm::vec3(u, -v, 1.0f); break;
                        case 5: R = glm::vec3(-u, -v, -1.0f); break;
                        }
                        R = glm::normalize(R);

                        glm::vec3 prefiltered(0.0f);
                        float totalWeight = 0.0f;
                        for (int i = 0; i < numSamples; ++i) {
                            glm::vec2 Xi = hammersley2d(i, numSamples);
                            glm::vec3 H = ImportanceSampleGGX(Xi, R, roughness);
                            glm::vec3 L = glm::normalize(2.0f * glm::dot(R, H) * H - R);
                            const float NdotL = std::max(glm::dot(R, L), 0.0f);
                            if (NdotL > 0.0f) {
                                prefiltered += SampleEquirectangular(inEquirect, L) * NdotL;
                                totalWeight += NdotL;
                            }
                        }
                        prefiltered = (totalWeight > 0.0f) ? (prefiltered / totalWeight) : prefiltered;
                        mipCube.setPixel(x, y, face, glm::vec4(prefiltered, 1.0f));
                    }
                }
            }
            mipMaps.push_back(mipCube);
        }
        return mipMaps;
    }

} // namespace graphics