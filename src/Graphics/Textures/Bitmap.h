#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <glm/glm.hpp>

//https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook

namespace graphics {

    enum class BitmapFormat { UnsignedByte, Float };
    enum class BitmapType { TwoD, Cube };

    class Bitmap {
    public:
        Bitmap() = default;
        Bitmap(int width, int height, int components, BitmapFormat format);
        Bitmap(int width, int height, int components, BitmapFormat format, const void* dataPtr);
        Bitmap(int width, int height, int depth, int components, BitmapFormat format);

        [[nodiscard]] bool LoadFromFile(const std::string& filePath,
            bool flipY = true,
            bool force4Ch = true);
        void SetType(BitmapType t) { type_ = t; }

        [[nodiscard]] int width() const noexcept { return width_; }
        [[nodiscard]] int height() const noexcept { return height_; }
        [[nodiscard]] int depth() const noexcept { return depth_; }
        [[nodiscard]] int components() const noexcept { return components_; }
        [[nodiscard]] BitmapFormat format() const noexcept { return format_; }
        [[nodiscard]] BitmapType type() const noexcept { return type_; }
        [[nodiscard]] bool IsHDR() const noexcept { return (format_ == BitmapFormat::Float); }

        [[nodiscard]] const std::vector<uint8_t>& data() const noexcept { return data_; }
        std::vector<uint8_t>& data() noexcept { return data_; }
        [[nodiscard]] const uint8_t* GetDataU8() const noexcept {
            return (format_ == BitmapFormat::UnsignedByte) ? data_.data() : nullptr;
        }
        uint8_t* GetDataU8() noexcept {
            return (format_ == BitmapFormat::UnsignedByte) ? data_.data() : nullptr;
        }
        [[nodiscard]] const float* GetDataFloat() const noexcept {
            return (format_ == BitmapFormat::Float)
                ? reinterpret_cast<const float*>(data_.data())
                : nullptr;
        }
        float* GetDataFloat() noexcept {
            return (format_ == BitmapFormat::Float)
                ? reinterpret_cast<float*>(data_.data())
                : nullptr;
        }

        [[nodiscard]] glm::vec4 getPixel(int x, int y) const;
        void setPixel(int x, int y, const glm::vec4& color);
        [[nodiscard]] glm::vec4 getPixel(int x, int y, int face) const;
        void setPixel(int x, int y, int face, const glm::vec4& color);

        static int getBytesPerComponent(BitmapFormat fmt);

    private:
        [[nodiscard]] glm::vec4 getPixelInternal(int x, int y) const;
        void setPixelInternal(int x, int y, const glm::vec4& color);
        void Init(int w, int h, int d, int comp, BitmapFormat fmt, BitmapType t);

        int width_ = 0;
        int height_ = 0;
        int depth_ = 1;
        int components_ = 4;
        BitmapFormat format_ = BitmapFormat::UnsignedByte;
        BitmapType type_ = BitmapType::TwoD;
        std::vector<uint8_t> data_;
    };

} // namespace graphics