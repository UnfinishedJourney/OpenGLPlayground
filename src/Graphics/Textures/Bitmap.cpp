#include "Bitmap.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace graphics {

    int Bitmap::getBytesPerComponent(BitmapFormat fmt) {
        switch (fmt) {
        case BitmapFormat::UnsignedByte: return 1;
        case BitmapFormat::Float:          return 4;
        default: return 0;
        }
    }

    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format)
        : width_(width), height_(height), components_(components), format_(format) {
        data_.resize(width_ * height_ * components_ * getBytesPerComponent(format_));
    }

    Bitmap::Bitmap(int width, int height, int components, BitmapFormat format, const void* ptr)
        : width_(width), height_(height), components_(components), format_(format) {
        data_.resize(width_ * height_ * components_ * getBytesPerComponent(format_));
        std::memcpy(data_.data(), ptr, data_.size());
    }

    Bitmap::Bitmap(int width, int height, int depth, int components, BitmapFormat format)
        : width_(width), height_(height), depth_(depth), components_(components), format_(format), type_(BitmapType::Cube) {
        data_.resize(width_ * height_ * depth_ * components_ * getBytesPerComponent(format_));
    }

    glm::vec4 Bitmap::getPixelInternal(int x, int y) const {
        int index = (y * width_ + x) * components_ * getBytesPerComponent(format_);
        if (format_ == BitmapFormat::Float) {
            const float* p = reinterpret_cast<const float*>(data_.data());
            return glm::vec4(
                components_ > 0 ? p[(y * width_ + x) * components_ + 0] : 0.0f,
                components_ > 1 ? p[(y * width_ + x) * components_ + 1] : 0.0f,
                components_ > 2 ? p[(y * width_ + x) * components_ + 2] : 0.0f,
                components_ > 3 ? p[(y * width_ + x) * components_ + 3] : 0.0f
            );
        }
        else {
            return glm::vec4(
                components_ > 0 ? data_[index + 0] / 255.0f : 0.0f,
                components_ > 1 ? data_[index + 1] / 255.0f : 0.0f,
                components_ > 2 ? data_[index + 2] / 255.0f : 0.0f,
                components_ > 3 ? data_[index + 3] / 255.0f : 0.0f
            );
        }
    }

    void Bitmap::setPixelInternal(int x, int y, const glm::vec4& color) {
        int index = (y * width_ + x) * components_ * getBytesPerComponent(format_);
        if (format_ == BitmapFormat::Float) {
            float* p = reinterpret_cast<float*>(data_.data());
            if (components_ > 0) p[(y * width_ + x) * components_ + 0] = color.x;
            if (components_ > 1) p[(y * width_ + x) * components_ + 1] = color.y;
            if (components_ > 2) p[(y * width_ + x) * components_ + 2] = color.z;
            if (components_ > 3) p[(y * width_ + x) * components_ + 3] = color.w;
        }
        else {
            if (components_ > 0) data_[index + 0] = static_cast<uint8_t>(glm::clamp(color.x, 0.0f, 1.0f) * 255.0f);
            if (components_ > 1) data_[index + 1] = static_cast<uint8_t>(glm::clamp(color.y, 0.0f, 1.0f) * 255.0f);
            if (components_ > 2) data_[index + 2] = static_cast<uint8_t>(glm::clamp(color.z, 0.0f, 1.0f) * 255.0f);
            if (components_ > 3) data_[index + 3] = static_cast<uint8_t>(glm::clamp(color.w, 0.0f, 1.0f) * 255.0f);
        }
    }

    void Bitmap::setPixel(int x, int y, const glm::vec4& color) {
        setPixelInternal(x, y, color);
    }

    glm::vec4 Bitmap::getPixel(int x, int y) const {
        return getPixelInternal(x, y);
    }

    void Bitmap::setPixel(int x, int y, int face, const glm::vec4& color) {
        if (type_ != BitmapType::Cube) {
            setPixel(x, y, color);
            return;
        }
        int facePixelCount = width_ * height_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);
        int index = pixelIndex * components_ * getBytesPerComponent(format_);
        if (format_ == BitmapFormat::Float) {
            float* p = reinterpret_cast<float*>(data_.data());
            if (components_ > 0) p[pixelIndex * components_ + 0] = color.x;
            if (components_ > 1) p[pixelIndex * components_ + 1] = color.y;
            if (components_ > 2) p[pixelIndex * components_ + 2] = color.z;
            if (components_ > 3) p[pixelIndex * components_ + 3] = color.w;
        }
        else {
            if (components_ > 0) data_[index + 0] = static_cast<uint8_t>(glm::clamp(color.x, 0.0f, 1.0f) * 255.0f);
            if (components_ > 1) data_[index + 1] = static_cast<uint8_t>(glm::clamp(color.y, 0.0f, 1.0f) * 255.0f);
            if (components_ > 2) data_[index + 2] = static_cast<uint8_t>(glm::clamp(color.z, 0.0f, 1.0f) * 255.0f);
            if (components_ > 3) data_[index + 3] = static_cast<uint8_t>(glm::clamp(color.w, 0.0f, 1.0f) * 255.0f);
        }
    }

    glm::vec4 Bitmap::getPixel(int x, int y, int face) const {
        if (type_ != BitmapType::Cube) {
            return getPixel(x, y);
        }
        int facePixelCount = width_ * height_;
        int pixelIndex = face * facePixelCount + (y * width_ + x);
        int index = pixelIndex * components_ * getBytesPerComponent(format_);
        if (format_ == BitmapFormat::Float) {
            const float* p = reinterpret_cast<const float*>(data_.data());
            return glm::vec4(
                components_ > 0 ? p[pixelIndex * components_ + 0] : 0.0f,
                components_ > 1 ? p[pixelIndex * components_ + 1] : 0.0f,
                components_ > 2 ? p[pixelIndex * components_ + 2] : 0.0f,
                components_ > 3 ? p[pixelIndex * components_ + 3] : 0.0f
            );
        }
        else {
            return glm::vec4(
                components_ > 0 ? data_[index + 0] / 255.0f : 0.0f,
                components_ > 1 ? data_[index + 1] / 255.0f : 0.0f,
                components_ > 2 ? data_[index + 2] / 255.0f : 0.0f,
                components_ > 3 ? data_[index + 3] / 255.0f : 0.0f
            );
        }
    }

} // namespace graphics