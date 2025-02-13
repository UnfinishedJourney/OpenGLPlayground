#include "EnvMapPreprocessor.h"
#include "Utilities/Logger.h"

// If you have not included <stb_image_write.h> or <stb_image_resize2.h> in another place:
#include <stb_image_write.h>
#include <stb_image_resize2.h>
#include <cmath>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

namespace graphics {

    namespace MathConstants {
        constexpr float PI = 3.14159265359f;
        constexpr float TWO_PI = 6.28318530718f;
    }

    // Utility clamp
    template <typename T>
    T clamp(T v, T lo, T hi) {
        return (v < lo) ? lo : ((v > hi) ? hi : v);
    }

    static float radicalInverse_VdC(uint32_t bits) {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10f;
    }

    static glm::vec2 hammersley2d(uint32_t i, uint32_t N) {
        return glm::vec2(float(i) / float(N), radicalInverse_VdC(i));
    }

    static glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize) {
        float A = 2.0f * float(i) / faceSize;
        float B = 2.0f * float(j) / faceSize;
        switch (faceID) {
        case 0: return glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
        case 1: return glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
        case 2: return glm::vec3(1.0f, A - 1.0f, 1.0f - B);
        case 3: return glm::vec3(1.0f - A, 1.0f, 1.0f - B);
        case 4: return glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
        case 5: return glm::vec3(1.0f - B, A - 1.0f, -1.0f);
        }
        return glm::vec3(0.0f);
    }

    static glm::vec3 ImportanceSampleGGX(const glm::vec2 Xi, const glm::vec3& N, float roughness) {
        float a = roughness * roughness;
        float phi = 2.0f * MathConstants::PI * Xi.x;
        float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
        float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
        glm::vec3 H(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

        glm::vec3 up = (fabs(N.z) < 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        glm::vec3 tangentX = glm::normalize(glm::cross(up, N));
        glm::vec3 tangentY = glm::cross(N, tangentX);

        return glm::normalize(tangentX * H.x + tangentY * H.y + N * H.z);
    }

    static glm::vec3 SampleEquirectangular(const Bitmap& envMap, const glm::vec3& dir) {
        float u = 0.5f + atan2(dir.z, dir.x) / (2.0f * MathConstants::PI);
        float v = acos(clamp(dir.y, -1.0f, 1.0f)) / MathConstants::PI;

        float xf = u * envMap.width();
        float yf = v * envMap.height();
        int x0 = clamp(int(std::floor(xf)), 0, envMap.width() - 1);
        int y0 = clamp(int(std::floor(yf)), 0, envMap.height() - 1);
        int x1 = clamp(x0 + 1, 0, envMap.width() - 1);
        int y1 = clamp(y0 + 1, 0, envMap.height() - 1);

        float s = xf - x0;
        float t = yf - y0;
        glm::vec4 A = envMap.getPixel(x0, y0);
        glm::vec4 B = envMap.getPixel(x1, y0);
        glm::vec4 C = envMap.getPixel(x0, y1);
        glm::vec4 D = envMap.getPixel(x1, y1);

        glm::vec4 color = A * (1.0f - s) * (1.0f - t)
            + B * s * (1.0f - t)
            + C * (1.0f - s) * t
            + D * s * t;
        return glm::vec3(color);
    }

    // -----------------------------------------------------------

    void EnvMapPreprocessor::SaveAsVerticalCross(const std::filesystem::path& texturePath,
        const std::filesystem::path& outPath) const
    {
        // If you still want to do direct stbi_loadf, you can, but let's just do:
        Bitmap bm;
        bm.LoadFromFile(texturePath.string(), /*flipY=*/false, /*force4Ch=*/false, /*isHDR=*/true);
        Bitmap vCross = ConvertEquirectangularMapToVerticalCross(bm);
        SaveAsHDR(vCross, outPath);
    }

    Bitmap EnvMapPreprocessor::LoadTexture(const std::filesystem::path& texturePath) const {
        // This is basically a convenience method:
        Bitmap tex;
        tex.LoadFromFile(texturePath.string(),
            /*flipY=*/false, /*force4Ch=*/true, /*isHDR=*/true);
        return tex;
    }

    Bitmap EnvMapPreprocessor::ConvertEquirectangularMapToVerticalCross(const Bitmap& b) const
    {
        if (b.type() != BitmapType::TwoD) {
            return Bitmap(); // return empty
        }
        int faceSize = b.width() / 4;
        int outW = faceSize * 3;
        int outH = faceSize * 4;

        Bitmap result(outW, outH, b.components(), b.format());
        // Offsets for each face in the vertical cross layout
        const std::array<glm::ivec2, 6> offsets = {
            glm::ivec2(faceSize, faceSize * 3),
            glm::ivec2(0,        faceSize),
            glm::ivec2(faceSize, faceSize),
            glm::ivec2(faceSize * 2, faceSize),
            glm::ivec2(faceSize, 0),
            glm::ivec2(faceSize, faceSize * 2)
        };
        int clampW = b.width() - 1;
        int clampH = b.height() - 1;

        for (int face = 0; face < 6; ++face) {
            for (int i = 0; i < faceSize; ++i) {
                for (int j = 0; j < faceSize; ++j) {
                    glm::vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
                    float theta = atan2(P.y, P.x);
                    float phi = atan2(P.z, hypot(P.x, P.y));
                    float Uf = 2.0f * faceSize * (theta + MathConstants::PI) / MathConstants::PI;
                    float Vf = 2.0f * faceSize * ((MathConstants::PI / 2.0f - phi) / MathConstants::PI);

                    int U1 = clamp(int(std::floor(Uf)), 0, clampW);
                    int V1 = clamp(int(std::floor(Vf)), 0, clampH);
                    int U2 = clamp(U1 + 1, 0, clampW);
                    int V2 = clamp(V1 + 1, 0, clampH);

                    float s = Uf - U1;
                    float t = Vf - V1;

                    glm::vec4 A = b.getPixel(U1, V1);
                    glm::vec4 B = b.getPixel(U2, V1);
                    glm::vec4 C = b.getPixel(U1, V2);
                    glm::vec4 D = b.getPixel(U2, V2);

                    glm::vec4 color = A * (1 - s) * (1 - t) + B * s * (1 - t) + C * (1 - s) * t + D * s * t;
                    result.setPixel(i + offsets[face].x, j + offsets[face].y, color);
                }
            }
        }
        return result;
    }

    Bitmap EnvMapPreprocessor::ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) const
    {
        int faceW = b.width() / 3;
        int faceH = b.height() / 4;
        // We'll store 6 faces in depth dimension
        Bitmap cube(faceW, faceH, 6, b.components(), b.format());

        int pixelSize = b.components() * Bitmap::getBytesPerComponent(b.format());
        int faceSizeBytes = faceW * faceH * pixelSize;
        const uint8_t* src = b.data().data();
        uint8_t* dst = cube.data().data();

        for (int face = 0; face < 6; ++face) {
            for (int j = 0; j < faceH; ++j) {
                for (int i = 0; i < faceW; ++i) {
                    // (Your original snippet used a certain layout for faces.)
                    int x = 0, y = 0;
                    switch (face) {
                    case 0: x = i;              y = faceH + j;         break;
                    case 1: x = 2 * faceW + i;    y = faceH + j;         break;
                    case 2: x = 2 * faceW - (i + 1); y = faceH - (j + 1);   break;
                    case 3: x = 2 * faceW - (i + 1); y = 3 * faceH - (j + 1); break;
                    case 4: x = 2 * faceW - (i + 1); y = b.height() - (j + 1); break;
                    case 5: x = faceW + i;      y = faceH + j;         break;
                    }
                    std::memcpy(dst, src + (y * b.width() + x) * pixelSize, pixelSize);
                    dst += pixelSize;
                }
            }
        }
        return cube;
    }

    Bitmap EnvMapPreprocessor::ComputeIrradianceEquirect(const Bitmap& inEquirect, int outW, int outH, int samples) const
    {
        if (inEquirect.format() != BitmapFormat::Float) {
            throw std::runtime_error("ComputeIrradianceEquirect: input must be float format!");
        }
        if (inEquirect.data().empty()) {
            throw std::runtime_error("ComputeIrradianceEquirect: input is empty!");
        }
        int srcW = inEquirect.width();
        int srcH = inEquirect.height();
        if (srcW != 2 * srcH) {
            throw std::runtime_error("ComputeIrradianceEquirect: equirect input should have width=2*height!");
        }
        Bitmap outBmp(outW, outH, 3, BitmapFormat::Float);
        const glm::vec3* srcData = reinterpret_cast<const glm::vec3*>(inEquirect.data().data());
        glm::vec3* dstData = reinterpret_cast<glm::vec3*>(outBmp.data().data());

        for (int y = 0; y < outH; ++y) {
            float theta1 = float(y) / outH * MathConstants::PI;
            for (int x = 0; x < outW; ++x) {
                float phi1 = float(x) / outW * MathConstants::TWO_PI;
                glm::vec3 V1(sin(theta1) * cos(phi1),
                    sin(theta1) * sin(phi1),
                    cos(theta1));
                glm::vec3 color(0.0f);
                float weight = 0.0f;
                for (int i = 0; i < samples; ++i) {
                    glm::vec2 Xi = hammersley2d(i, samples);
                    int x1 = int(std::floor(Xi.x * srcW));
                    int y1 = int(std::floor(Xi.y * srcH));
                    float theta2 = float(y1) / srcH * MathConstants::PI;
                    float phi2 = float(x1) / srcW * MathConstants::TWO_PI;
                    glm::vec3 V2(sin(theta2) * cos(phi2),
                        sin(theta2) * sin(phi2),
                        cos(theta2));
                    float D = std::max(glm::dot(V1, V2), 0.0f);
                    if (D > 0.01f) {
                        color += srcData[y1 * srcW + x1] * D;
                        weight += D;
                    }
                }
                dstData[y * outW + x] = (weight > 0.0f) ? color / weight : color;
            }
        }
        return outBmp;
    }

    void EnvMapPreprocessor::SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) const
    {
        if (image.format() != BitmapFormat::Float) {
            throw std::runtime_error("SaveAsHDR: image must be float format!");
        }
        int w = image.width();
        int h = image.height();
        int c = image.components();
        if (c < 3 || c>4) {
            throw std::runtime_error("SaveAsHDR: only 3 or 4 components supported!");
        }
        if (image.depth() != 1) {
            throw std::runtime_error("SaveAsHDR: cannot save 3D/cubemap directly; flatten first!");
        }
        const float* ptr = reinterpret_cast<const float*>(image.data().data());
        if (!stbi_write_hdr(outPath.string().c_str(), w, h, c, ptr)) {
            throw std::runtime_error("stbi_write_hdr failed!");
        }
    }

    void EnvMapPreprocessor::SaveAsLDR(const Bitmap& image, const std::filesystem::path& outPath) const
    {
        // We'll just do a naive gamma-encode and store as PNG
        int w = image.width();
        int h = image.height();
        int comp = image.components();
        std::vector<uint8_t> ldrData(w * h * comp);

        if (image.format() == BitmapFormat::Float) {
            // Convert float -> 8bit
            const float* src = reinterpret_cast<const float*>(image.data().data());
            for (int i = 0; i < w * h * comp; ++i) {
                float val = std::clamp(src[i], 0.0f, 1.0f);
                val = powf(val, 1.0f / 2.2f); // gamma
                ldrData[i] = static_cast<uint8_t>(val * 255.0f);
            }
        }
        else {
            // Already 8bit
            ldrData = image.data();
        }

        if (!stbi_write_png(outPath.string().c_str(), w, h, comp, ldrData.data(), w * comp)) {
            throw std::runtime_error("stbi_write_png failed in SaveAsLDR!");
        }
    }

    void EnvMapPreprocessor::SaveFacesToDiskLDR(const Bitmap& cubeMap,
        const std::array<std::filesystem::path, 6>& facePaths,
        const std::string& prefix) const
    {
        if (cubeMap.depth() != 6) {
            Logger::GetLogger()->error("SaveFacesToDiskLDR: expected depth=6, got {}.", cubeMap.depth());
            return;
        }
        int faceW = cubeMap.width();
        int faceH = cubeMap.height();
        int comp = cubeMap.components();
        int pixelSize = comp * Bitmap::getBytesPerComponent(cubeMap.format());
        int faceSizeBytes = faceW * faceH * pixelSize;

        for (int face = 0; face < 6; ++face) {
            Bitmap faceBmp(faceW, faceH, comp, cubeMap.format());
            const uint8_t* src = cubeMap.data().data() + face * faceSizeBytes;
            std::memcpy(faceBmp.data().data(), src, faceSizeBytes);

            try {
                SaveAsLDR(faceBmp, facePaths[face]);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("SaveFacesToDiskLDR: face {}: {}", prefix + std::to_string(face), e.what());
            }
        }
    }

    std::vector<Bitmap> EnvMapPreprocessor::ComputePrefilteredCubemap(const Bitmap& inEquirect,
        int baseFaceSize,
        int numSamples) const
    {
        int mipLevels = int(std::floor(std::log2(float(baseFaceSize)))) + 1;
        std::vector<Bitmap> mipMaps;
        mipMaps.reserve(mipLevels);

        for (int mip = 0; mip < mipLevels; ++mip) {
            int faceSize = baseFaceSize >> mip;
            Bitmap mipCube(faceSize, faceSize, 6, inEquirect.components(), inEquirect.format());
            mipCube.SetType(BitmapType::Cube);

            float roughness = (mipLevels > 1) ? float(mip) / float(mipLevels - 1) : 0.0f;
            for (int face = 0; face < 6; ++face) {
                for (int y = 0; y < faceSize; ++y) {
                    for (int x = 0; x < faceSize; ++x) {
                        float u = (2.0f * (x + 0.5f) / faceSize) - 1.0f;
                        float v = (2.0f * (y + 0.5f) / faceSize) - 1.0f;
                        glm::vec3 R;
                        // face orientation
                        switch (face) {
                        case 0: R = glm::vec3(1.0f, -v, -u);   break;
                        case 1: R = glm::vec3(-1.0f, -v, u);  break;
                        case 2: R = glm::vec3(u, 1.0f, v);   break;
                        case 3: R = glm::vec3(u, -1.0f, -v);   break;
                        case 4: R = glm::vec3(u, -v, 1.0f);   break;
                        case 5: R = glm::vec3(-u, -v, -1.0f);  break;
                        }
                        R = glm::normalize(R);

                        glm::vec3 prefiltered(0.0f);
                        float totalWeight = 0.0f;
                        for (int i = 0; i < numSamples; ++i) {
                            glm::vec2 Xi = hammersley2d(i, numSamples);
                            glm::vec3 H = ImportanceSampleGGX(Xi, R, roughness);
                            glm::vec3 L = glm::normalize(2.0f * glm::dot(R, H) * H - R);
                            float NdotL = std::max(glm::dot(R, L), 0.0f);
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