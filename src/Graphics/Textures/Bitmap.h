#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <glm/glm.hpp>

//modified
//https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook

//need to potentially unite it with texturedata
namespace Graphics {

    enum class BitmapType {
        TwoD,
        Cube
    };

    enum class BitmapFormat {
        UnsignedByte,
        Float
    };

    class Bitmap {
    public:
        // Constructors for 2D images
        Bitmap() = default;
        Bitmap(int width, int height, int components, BitmapFormat format);
        Bitmap(int width, int height, int components, BitmapFormat format, const void* ptr);

        // Constructor for images with multiple layers (e.g. cubemaps, where depth==6)
        Bitmap(int width, int height, int depth, int components, BitmapFormat format);

        // Getters
        int width() const { return width_; }
        int height() const { return height_; }
        int depth() const { return depth_; }
        int components() const { return components_; }
        BitmapFormat format() const { return format_; }
        BitmapType type() const { return type_; }
        const std::vector<uint8_t>& data() const { return data_; }
        std::vector<uint8_t>& data() { return data_; }

        // Returns number of bytes per component.
        static int getBytesPerComponent(BitmapFormat fmt);

        // 2D pixel access
        void setPixel(int x, int y, const glm::vec4& color);
        glm::vec4 getPixel(int x, int y) const;

        // Cubemap pixel access (if type() == Cube)
        void setPixel(int x, int y, int face, const glm::vec4& color);
        glm::vec4 getPixel(int x, int y, int face) const;
        BitmapType type_ = BitmapType::TwoD;

    private:
        int width_ = 0;
        int height_ = 0;
        int depth_ = 1;  // For 2D images, depth is 1; for cubemaps, depth==6.
        int components_ = 3;
        BitmapFormat format_ = BitmapFormat::UnsignedByte;
        std::vector<uint8_t> data_;

        // Helper methods for 2D pixel access (format-specific)
        glm::vec4 getPixelInternal(int x, int y) const;
        void setPixelInternal(int x, int y, const glm::vec4& color);
    };

} // namespace Graphics