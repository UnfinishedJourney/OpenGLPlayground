#pragma once
#include <filesystem>
#include <vector>
#include <array>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <string>
#include <stb_image_write.h>
#include "Utilities/Logger.h"
#include "TextureConfig.h"
#include "Bitmap.h"
#include <glad/glad.h>

namespace graphics {

    inline GLenum ResolveInternalFormat(const TextureConfig& config, int channels) {
        GLenum chosen = config.internal_format_;
        if (config.is_hdr_) {
            if (chosen == GL_RGBA8 || chosen == GL_SRGB8_ALPHA8) chosen = GL_RGBA16F;
        }
        else if (config.is_srgb_) {
            if (chosen == GL_RGBA8) chosen = GL_SRGB8_ALPHA8;
        }
        return chosen;
    }

    inline GLenum DataTypeForHDR(bool is_hdr) {
        return is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    }

    inline void SetupTextureParameters(GLuint texID, const TextureConfig& cfg, bool isCubeMap = false) {
        if (cfg.generate_mips_) glGenerateTextureMipmap(texID);
        glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, cfg.min_filter_);
        glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, cfg.mag_filter_);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_S, cfg.wrap_s_);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_T, cfg.wrap_t_);
        if (isCubeMap) glTextureParameteri(texID, GL_TEXTURE_WRAP_R, cfg.wrap_r_);
        if (cfg.use_anisotropy_) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            glTextureParameterf(texID, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
        }
    }

    inline bool SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) {
        if (image.format() != BitmapFormat::Float) {
            Logger::GetLogger()->error("SaveAsHDR: Image not float format.");
            return false;
        }
        int w = image.width(), h = image.height(), c = image.components();
        if (c < 3 || c > 4) {
            Logger::GetLogger()->error("SaveAsHDR: Only 3 or 4 components supported (found {}).", c);
            return false;
        }
        if (image.depth() != 1) {
            Logger::GetLogger()->error("SaveAsHDR: Cannot save a cubemap (depth={}).", image.depth());
            return false;
        }
        const float* ptr = image.GetDataFloat();
        if (!ptr) {
            Logger::GetLogger()->error("SaveAsHDR: No float data.");
            return false;
        }
        if (!stbi_write_hdr(outPath.string().c_str(), w, h, c, ptr)) {
            Logger::GetLogger()->error("SaveAsHDR: stbi_write_hdr failed for '{}'.", outPath.string());
            return false;
        }
        Logger::GetLogger()->info("SaveAsHDR: Wrote '{}' ({}x{}, {} comps).", outPath.string(), w, h, c);
        return true;
    }

    inline bool SaveAsLDR(const Bitmap& image, const std::filesystem::path& outPath) {
        int w = image.width(), h = image.height(), c = image.components();
        if (w <= 0 || h <= 0 || c <= 0) {
            Logger::GetLogger()->error("SaveAsLDR: Invalid dimensions/components.");
            return false;
        }
        std::vector<uint8_t> ldrData(w * h * c);
        if (image.format() == BitmapFormat::Float) {
            const float* src = image.GetDataFloat();
            if (!src) {
                Logger::GetLogger()->error("SaveAsLDR: No float data.");
                return false;
            }
            for (int i = 0; i < w * h * c; ++i) {
                float val = std::clamp(src[i], 0.0f, 1.0f);
                ldrData[i] = static_cast<uint8_t>(std::pow(val, 1.0f / 2.2f) * 255.0f);
            }
        }
        else {
            ldrData = image.data();
        }
        if (!stbi_write_png(outPath.string().c_str(), w, h, c, ldrData.data(), w * c)) {
            Logger::GetLogger()->error("SaveAsLDR: stbi_write_png failed for '{}'.", outPath.string());
            return false;
        }
        Logger::GetLogger()->info("SaveAsLDR: Wrote '{}' ({}x{}, {} comps).", outPath.string(), w, h, c);
        return true;
    }

    inline bool SaveFacesToDiskLDR(const Bitmap& cubeMap, const std::array<std::filesystem::path, 6>& facePaths, const std::string& prefix = "") {
        if (cubeMap.depth() != 6) {
            Logger::GetLogger()->error("SaveFacesToDiskLDR: Expected cubemap with depth=6 (got {}).", cubeMap.depth());
            return false;
        }
        int faceW = cubeMap.width(), faceH = cubeMap.height(), comp = cubeMap.components();
        int pixelSize = comp * Bitmap::getBytesPerComponent(cubeMap.format());
        int faceSizeBytes = faceW * faceH * pixelSize;
        bool allOkay = true;
        for (int face = 0; face < 6; ++face) {
            Bitmap faceBmp(faceW, faceH, comp, cubeMap.format());
            const uint8_t* src = cubeMap.data().data() + face * faceSizeBytes;
            std::memcpy(faceBmp.data().data(), src, faceSizeBytes);
            if (!SaveAsLDR(faceBmp, facePaths[face])) {
                Logger::GetLogger()->error("SaveFacesToDiskLDR: Face {} failed for '{}'.", prefix + std::to_string(face), facePaths[face].string());
                allOkay = false;
            }
            else {
                Logger::GetLogger()->info("SaveFacesToDiskLDR: Face {} -> '{}'.", prefix + std::to_string(face), facePaths[face].string());
            }
        }
        return allOkay;
    }

} // namespace graphics