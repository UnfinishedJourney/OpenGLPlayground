#include "Bitmap.h"
#include <stb_image.h>
#include "Utilities/Logger.h"
#include <filesystem>

namespace graphics {

    int Bitmap::getBytesPerComponent(BitmapFormat fmt) {
        switch (fmt) {
        case BitmapFormat::UnsignedByte: return 1;
        case BitmapFormat::Float:        return 4;
        }
        return 0;
    }

    void Bitmap::Init(int w, int h, int d, int comp, BitmapFormat fmt, BitmapType t) {
        if (w <= 0 || h <= 0 || d <= 0 || comp <= 0) {
            const std::string err = "Bitmap::Init: Invalid dimensions or components (" +
                std::to_string(w) + "x" + std::to_string(h) +
                "x" + std::to_string(d) + ", comp=" + std::to_string(comp) + ")";
            Logger::GetLogger()->error(err);
            throw std::runtime_error(err);
        }
        width_ = w; height_ = h; depth_ = d; components_ = comp;
        format_ = fmt; type_ = t;
        if (depth_ == 6 && type_ == BitmapType::TwoD)
            type_ = BitmapType::Cube;
        size_t totalSize = static_cast<size_t>(w) * h * d * comp * getBytesPerComponent(fmt);
        data_.resize(totalSize);
        Logger::GetLogger()->info("Bitmap::Init: Allocated {} bytes ({}x{}x{}, {} comps, {}).",
            totalSize, w, h, d, comp,
            (fmt == BitmapFormat::Float ? "float" : "ubyte"));
    }

    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format) {
        Init(width, height, 1, components, format, BitmapType::TwoD);
    }

    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format, const void* dataPtr) {
        Init(width, height, 1, components, format, BitmapType::TwoD);
        if (dataPtr) {
            std::memcpy(data_.data(), dataPtr, data_.size());
            Logger::GetLogger()->info("Bitmap(2D+data): Copied initial data ({} bytes).", data_.size());
        }
    }

    Bitmap::Bitmap(int width, int height, int depth, int components, BitmapFormat format) {
        BitmapType t = (depth == 6) ? BitmapType::Cube : BitmapType::TwoD;
        Init(width, height, depth, components, format, t);
    }

    bool Bitmap::LoadFromFile(const std::string& filePath,
        bool flipY,
        bool force4Ch) {
        stbi_set_flip_vertically_on_load(flipY ? 1 : 0);
        const bool isHDR = (stbi_is_hdr(filePath.c_str()) != 0);
        const int desiredCh = force4Ch ? 4 : 0;
        int w = 0, h = 0, n = 0;
        void* stbiData = nullptr;
        if (isHDR) {
            float* fPtr = stbi_loadf(filePath.c_str(), &w, &h, &n, desiredCh);
            if (!fPtr) {
                const std::string err = "Bitmap::LoadFromFile: stbi_loadf failed on '" + filePath + "'";
                Logger::GetLogger()->error(err);
                throw std::runtime_error(err);
            }
            stbiData = fPtr;
        }
        else {
            unsigned char* cPtr = stbi_load(filePath.c_str(), &w, &h, &n, desiredCh);
            if (!cPtr) {
                const std::string err = "Bitmap::LoadFromFile: stbi_load failed on '" + filePath + "'";
                Logger::GetLogger()->error(err);
                throw std::runtime_error(err);
            }
            stbiData = cPtr;
        }
        if (desiredCh != 0)
            n = desiredCh;
        BitmapFormat finalFmt = isHDR ? BitmapFormat::Float : BitmapFormat::UnsignedByte;
        Init(w, h, 1, n, finalFmt, BitmapType::TwoD);
        std::memcpy(data_.data(), stbiData, data_.size());
        stbi_image_free(stbiData);
        Logger::GetLogger()->info("Bitmap::LoadFromFile: Loaded '{}' ({}x{}, {} comps, flipY={}, HDR={}).",
            filePath, w, h, n, flipY, (finalFmt == BitmapFormat::Float));
        return true;
    }

    glm::vec4 Bitmap::getPixelInternal(int x, int y) const {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            Logger::GetLogger()->warn("Bitmap::getPixelInternal: Out-of-bounds access at ({}, {})", x, y);
            return glm::vec4(0.0f);
        }
        int bytesPerComp = getBytesPerComponent(format_);
        int pixelIndex = (y * width_ + x) * components_;
        int byteIndex = pixelIndex * bytesPerComp;
        if (format_ == BitmapFormat::Float) {
            const float* fData = reinterpret_cast<const float*>(data_.data());
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c)
                col[c] = fData[pixelIndex + c];
            return col;
        }
        else {
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c)
                col[c] = data_[byteIndex + c] / 255.0f;
            return col;
        }
    }

    void Bitmap::setPixelInternal(int x, int y, const glm::vec4& color) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            Logger::GetLogger()->warn("Bitmap::setPixelInternal: Out-of-bounds access at ({}, {})", x, y);
            return;
        }
        int bytesPerComp = getBytesPerComponent(format_);
        int pixelIndex = (y * width_ + x) * components_;
        int byteIndex = pixelIndex * bytesPerComp;
        if (format_ == BitmapFormat::Float) {
            float* fData = reinterpret_cast<float*>(data_.data());
            for (int c = 0; c < components_; ++c)
                fData[pixelIndex + c] = color[c];
        }
        else {
            for (int c = 0; c < components_; ++c) {
                float val = std::clamp(color[c], 0.0f, 1.0f);
                data_[byteIndex + c] = static_cast<uint8_t>(val * 255.0f);
            }
        }
    }

    glm::vec4 Bitmap::getPixel(int x, int y) const {
        return getPixelInternal(x, y);
    }

    void Bitmap::setPixel(int x, int y, const glm::vec4& color) {
        setPixelInternal(x, y, color);
    }

    void Bitmap::setPixel(int x, int y, int face, const glm::vec4& color) {
        if (type_ != BitmapType::Cube || depth_ != 6) {
            setPixel(x, y, color);
            return;
        }
        if (face < 0 || face >= 6) {
            Logger::GetLogger()->warn("Bitmap::setPixel(cubemap): Invalid face index {}.", face);
            return;
        }
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            Logger::GetLogger()->warn("Bitmap::setPixel(cubemap): Out-of-bounds access at ({}, {}).", x, y);
            return;
        }
        int facePixelCount = width_ * height_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);
        int bytesPerComp = getBytesPerComponent(format_);
        int byteIndex = pixelIndex * components_ * bytesPerComp;
        if (format_ == BitmapFormat::Float) {
            float* fData = reinterpret_cast<float*>(data_.data());
            for (int c = 0; c < components_; ++c)
                fData[pixelIndex * components_ + c] = color[c];
        }
        else {
            for (int c = 0; c < components_; ++c) {
                float val = std::clamp(color[c], 0.0f, 1.0f);
                data_[byteIndex + c] = static_cast<uint8_t>(val * 255.0f);
            }
        }
    }

    glm::vec4 Bitmap::getPixel(int x, int y, int face) const {
        if (type_ != BitmapType::Cube || depth_ != 6)
            return getPixel(x, y);
        if (face < 0 || face >= 6) {
            Logger::GetLogger()->warn("Bitmap::getPixel(cubemap): Invalid face {}.", face);
            return glm::vec4(0.0f);
        }
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            Logger::GetLogger()->warn("Bitmap::getPixel(cubemap): Out-of-bounds access at ({}, {}).", x, y);
            return glm::vec4(0.0f);
        }
        int facePixelCount = width_ * height_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);
        int bytesPerComp = getBytesPerComponent(format_);
        int byteIndex = pixelIndex * components_ * bytesPerComp;
        if (format_ == BitmapFormat::Float) {
            const float* fData = reinterpret_cast<const float*>(data_.data());
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c)
                col[c] = fData[pixelIndex * components_ + c];
            return col;
        }
        else {
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c)
                col[c] = data_[byteIndex + c] / 255.0f;
            return col;
        }
    }

} // namespace graphics