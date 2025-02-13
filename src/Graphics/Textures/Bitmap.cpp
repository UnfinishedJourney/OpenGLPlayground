#include "Bitmap.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <cmath>

#include <stb_image.h>

namespace graphics {

    int Bitmap::getBytesPerComponent(BitmapFormat fmt) {
        switch (fmt) {
        case BitmapFormat::UnsignedByte: return 1;
        case BitmapFormat::Float:        return 4;
        }
        return 0; // unreachable
    }

    // --------------------------------------
    //  Constructors
    // --------------------------------------
    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format)
        : width_(width)
        , height_(height)
        , depth_(1)
        , components_(components)
        , format_(format)
        , type_(BitmapType::TwoD)
    {
        if (width_ <= 0 || height_ <= 0 || components_ <= 0) {
            throw std::runtime_error("Bitmap::Bitmap: invalid dimensions or components!");
        }
        size_t totalSize = size_t(width_) * size_t(height_) *
            size_t(components_) * getBytesPerComponent(format_);
        data_.resize(totalSize);
    }

    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format, const void* dataPtr)
        : Bitmap(width, height, components, format)
    {
        if (dataPtr) {
            std::memcpy(data_.data(), dataPtr, data_.size());
        }
    }

    Bitmap::Bitmap(int width, int height, int depth, int components, BitmapFormat format)
        : width_(width)
        , height_(height)
        , depth_(depth)
        , components_(components)
        , format_(format)
        , type_(BitmapType::TwoD)
    {
        if (depth_ > 1) {
            type_ = BitmapType::Cube; // or some generic "3D" if you prefer
        }
        if (width_ <= 0 || height_ <= 0 || depth_ <= 0 || components_ <= 0) {
            throw std::runtime_error("Bitmap::Bitmap(3D/cube): invalid arguments!");
        }
        size_t totalSize = size_t(width_) * size_t(height_) * size_t(depth_) *
            size_t(components_) * getBytesPerComponent(format_);
        data_.resize(totalSize);
    }

    // --------------------------------------
    //  LoadFromFile (unifies old TextureData usage)
    // --------------------------------------
    bool Bitmap::LoadFromFile(const std::string& filePath, bool flipY, bool force4Ch, bool isHDR)
    {
        stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

        int desiredCh = force4Ch ? 4 : 0;
        int w = 0, h = 0, n = 0;
        void* stbiData = nullptr;

        if (isHDR) {
            float* fPtr = stbi_loadf(filePath.c_str(), &w, &h, &n, desiredCh);
            if (!fPtr) {
                throw std::runtime_error("Bitmap::LoadFromFile: failed to load HDR: " + filePath);
            }
            stbiData = fPtr;
        }
        else {
            unsigned char* cPtr = stbi_load(filePath.c_str(), &w, &h, &n, desiredCh);
            if (!cPtr) {
                throw std::runtime_error("Bitmap::LoadFromFile: failed to load LDR: " + filePath);
            }
            stbiData = cPtr;
        }

        if (desiredCh != 0) {
            n = desiredCh;
        }

        width_ = w;
        height_ = h;
        depth_ = 1;
        type_ = BitmapType::TwoD;
        components_ = n;
        format_ = isHDR ? BitmapFormat::Float : BitmapFormat::UnsignedByte;

        size_t totalSize = size_t(width_) * size_t(height_) *
            size_t(components_) * getBytesPerComponent(format_);
        data_.resize(totalSize);

        std::memcpy(data_.data(), stbiData, data_.size());
        stbi_image_free(stbiData);

        return true; // loaded successfully
    }

    // --------------------------------------
    //  Pixel read/write (2D)
    // --------------------------------------
    glm::vec4 Bitmap::getPixelInternal(int x, int y) const
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            return glm::vec4(0.0f);
        }
        int bytesPerComp = getBytesPerComponent(format_);
        int pixelIndex = (y * width_ + x) * components_;
        int byteIndex = pixelIndex * bytesPerComp;

        if (format_ == BitmapFormat::Float) {
            const float* fData = reinterpret_cast<const float*>(data_.data());
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c) {
                col[c] = fData[pixelIndex + c];
            }
            return col;
        }
        else {
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c) {
                col[c] = data_[byteIndex + c] / 255.0f;
            }
            return col;
        }
    }

    void Bitmap::setPixelInternal(int x, int y, const glm::vec4& color)
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            return; // out of bounds
        }
        int bytesPerComp = getBytesPerComponent(format_);
        int pixelIndex = (y * width_ + x) * components_;
        int byteIndex = pixelIndex * bytesPerComp;

        if (format_ == BitmapFormat::Float) {
            float* fData = reinterpret_cast<float*>(data_.data());
            for (int c = 0; c < components_; ++c) {
                float val = (c == 0) ? color.x
                    : (c == 1) ? color.y
                    : (c == 2) ? color.z
                    : color.w;
                fData[pixelIndex + c] = val;
            }
        }
        else {
            for (int c = 0; c < components_; ++c) {
                float val = (c == 0) ? color.x
                    : (c == 1) ? color.y
                    : (c == 2) ? color.z
                    : color.w;
                val = std::clamp(val, 0.0f, 1.0f);
                data_[byteIndex + c] = static_cast<uint8_t>(val * 255.0f);
            }
        }
    }

    glm::vec4 Bitmap::getPixel(int x, int y) const
    {
        return getPixelInternal(x, y);
    }

    void Bitmap::setPixel(int x, int y, const glm::vec4& color)
    {
        setPixelInternal(x, y, color);
    }

    // --------------------------------------
    //  Pixel read/write (cubemap face)
    // --------------------------------------
    void Bitmap::setPixel(int x, int y, int face, const glm::vec4& color)
    {
        if (type_ != BitmapType::Cube || depth_ != 6) {
            // fallback to 2D usage if not actually a cube
            setPixel(x, y, color);
            return;
        }
        // face index
        if (face < 0 || face >= 6) return;
        int facePixelCount = width_ * height_;
        int totalComp = facePixelCount * components_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);

        int bytesPerComp = getBytesPerComponent(format_);
        int byteIndex = pixelIndex * components_ * bytesPerComp;

        if (format_ == BitmapFormat::Float) {
            float* fData = reinterpret_cast<float*>(data_.data());
            for (int c = 0; c < components_; ++c) {
                float val = (c == 0) ? color.x
                    : (c == 1) ? color.y
                    : (c == 2) ? color.z
                    : color.w;
                fData[pixelIndex * components_ + c] = val;
            }
        }
        else {
            for (int c = 0; c < components_; ++c) {
                float val = (c == 0) ? color.x
                    : (c == 1) ? color.y
                    : (c == 2) ? color.z
                    : color.w;
                val = std::clamp(val, 0.0f, 1.0f);
                data_[byteIndex + c] = static_cast<uint8_t>(val * 255.0f);
            }
        }
    }

    glm::vec4 Bitmap::getPixel(int x, int y, int face) const
    {
        if (type_ != BitmapType::Cube || depth_ != 6) {
            // fallback
            return getPixel(x, y);
        }
        if (face < 0 || face >= 6) {
            return glm::vec4(0.0f);
        }
        int facePixelCount = width_ * height_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);

        int bytesPerComp = getBytesPerComponent(format_);
        int byteIndex = pixelIndex * components_ * bytesPerComp;

        if (format_ == BitmapFormat::Float) {
            const float* fData = reinterpret_cast<const float*>(data_.data());
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c) {
                col[c] = fData[pixelIndex * components_ + c];
            }
            return col;
        }
        else {
            glm::vec4 col(0.0f);
            for (int c = 0; c < components_; ++c) {
                col[c] = data_[byteIndex + c] / 255.0f;
            }
            return col;
        }
    }

} // namespace graphics