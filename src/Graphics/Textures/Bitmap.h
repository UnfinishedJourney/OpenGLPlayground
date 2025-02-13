#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace graphics {

    /// Distinguishes how pixel data is stored internally (float or byte).
    enum class BitmapFormat {
        UnsignedByte,
        Float
    };

    /// Whether this bitmap is a 2D image or a cubemap / multi-layer (depth=6).
    enum class BitmapType {
        TwoD,
        Cube
    };

    /**
     * @brief A flexible class for in-memory image data.
     *
     * - Can load from file (HDR or LDR) using stb_image.
     * - Stores float or byte data internally.
     * - Supports multiple layers (depth=6 for cubemaps).
     * - Provides get/set pixel methods for direct access.
     */
    class Bitmap {
    public:
        // 1) Constructors
        Bitmap() = default;  // Empty

        /// 2D constructor (allocates memory)
        Bitmap(int width, int height, int components, BitmapFormat format);

        /// 2D constructor with initial data
        Bitmap(int width, int height, int components, BitmapFormat format, const void* dataPtr);

        /// Cubemap (or multi-layer) constructor
        Bitmap(int width, int height, int depth, int components, BitmapFormat format);

        // 2) File loading: unify old TextureData logic here
        bool LoadFromFile(const std::string& filePath,
            bool flipY = true,
            bool force4Ch = true,
            bool isHDR = false);

        void SetType(BitmapType t) { type_ = t; }

        // 3) Basic getters
        int           width()      const { return width_; }
        int           height()     const { return height_; }
        int           depth()      const { return depth_; }
        int           components() const { return components_; }
        BitmapFormat  format()     const { return format_; }
        BitmapType    type()       const { return type_; }
        bool          IsHDR()      const { return (format_ == BitmapFormat::Float); }

        // Raw data buffer (byte-based). For float data, reinterpret cast needed.
        const std::vector<uint8_t>& data() const { return data_; }
        std::vector<uint8_t>& data() { return data_; }

        // Convenient typed pointers (nullptr if format mismatch).
        const uint8_t* GetDataU8() const {
            return (format_ == BitmapFormat::UnsignedByte) ? data_.data() : nullptr;
        }
        uint8_t* GetDataU8() {
            return (format_ == BitmapFormat::UnsignedByte) ? data_.data() : nullptr;
        }
        const float* GetDataFloat() const {
            return (format_ == BitmapFormat::Float)
                ? reinterpret_cast<const float*>(data_.data())
                : nullptr;
        }
        float* GetDataFloat() {
            return (format_ == BitmapFormat::Float)
                ? reinterpret_cast<float*>(data_.data())
                : nullptr;
        }

        // 4) Pixel access
        // For 2D images:
        void      setPixel(int x, int y, const glm::vec4& color);
        glm::vec4 getPixel(int x, int y) const;

        // For cubemaps (depth=6). If not a cubemap, fallback to 2D usage.
        void      setPixel(int x, int y, int face, const glm::vec4& color);
        glm::vec4 getPixel(int x, int y, int face) const;

        // 5) Utility
        static int getBytesPerComponent(BitmapFormat fmt);

    private:
        // Helpers for internal pixel read/write
        glm::vec4 getPixelInternal(int x, int y) const;
        void      setPixelInternal(int x, int y, const glm::vec4& color);

    private:
        int width_ = 0;
        int height_ = 0;
        int depth_ = 1; // 1 for 2D, 6 for cubemap
        int components_ = 4; // default e.g. RGBA
        BitmapFormat format_ = BitmapFormat::UnsignedByte;
        BitmapType   type_ = BitmapType::TwoD;

        std::vector<uint8_t> data_; // Raw bytes
    };

} // namespace graphics