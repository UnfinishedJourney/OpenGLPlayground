#pragma once

#include <string.h>
#include <vector>
#include <glm/glm.hpp>

/**
 * @enum eBitmapType
 * @brief Indicate if this Bitmap is 2D or a Cube (6 slices in depth).
 */
enum eBitmapType
{
    eBitmapType_2D,
    eBitmapType_Cube
};

/**
 * @enum eBitmapFormat
 * @brief Indicate whether each component is stored as an 8-bit unsigned byte or a 32-bit float.
 */
enum eBitmapFormat
{
    eBitmapFormat_UnsignedByte,
    eBitmapFormat_Float,
};

/**
 * @struct Bitmap
 * @brief Represents an image (2D or 3D) in either 8-bit or float format, with R/G/B/(A) channels.
 */
struct Bitmap
{
    Bitmap() = default;

    /**
     * @brief Constructs a 2D Bitmap with given width, height, number of components, and format.
     */
    Bitmap(int w, int h, int comp, eBitmapFormat fmt)
        : w_(w), h_(h), comp_(comp), fmt_(fmt),
        data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }

    /**
     * @brief Constructs a 3D Bitmap (width x height x depth slices).
     */
    Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt)
        : w_(w), h_(h), d_(d), comp_(comp), fmt_(fmt),
        data_(w* h* d* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }

    /**
     * @brief Constructs a 2D Bitmap and copies initial data from 'ptr'.
     */
    Bitmap(int w, int h, int comp, eBitmapFormat fmt, const void* ptr)
        : w_(w), h_(h), comp_(comp), fmt_(fmt),
        data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
        memcpy(data_.data(), ptr, data_.size());
    }

    // Dimensions
    int w_ = 0;
    int h_ = 0;
    int d_ = 1;  // If >1, can store multiple slices/faces
    int comp_ = 3;

    eBitmapFormat fmt_ = eBitmapFormat_UnsignedByte;
    eBitmapType   type_ = eBitmapType_2D;

    // Actual pixel data (size = w_ * h_ * d_ * comp_ * bytesPerComponent)
    std::vector<uint8_t> data_;

    /**
     * @brief Returns the number of bytes each channel takes (1 for U8, 4 for float).
     */
    static int getBytesPerComponent(eBitmapFormat fmt)
    {
        if (fmt == eBitmapFormat_UnsignedByte) return 1;
        if (fmt == eBitmapFormat_Float)        return 4;
        return 0; // unknown
    }

    // -----------------------------------
    // 2D Get/Set
    // -----------------------------------
    void setPixel(int x, int y, const glm::vec4& c)
    {
        (this->*setPixelFunc)(x, y, c);
    }
    glm::vec4 getPixel(int x, int y) const
    {
        return (this->*getPixelFunc)(x, y);
    }

    // -----------------------------------
    // Optional 3D access for d_ > 1
    // -----------------------------------
    // If you want to store 6 faces in a single Bitmap, you can do:
    void setPixel3D(int x, int y, int z, const glm::vec4& c)
    {
        (this->*setPixel3DFunc)(x, y, z, c);
    }
    glm::vec4 getPixel3D(int x, int y, int z) const
    {
        return (this->*getPixel3DFunc)(x, y, z);
    }

private:
    // Function pointer types for 2D and 3D
    using setPixel_t = void(Bitmap::*)(int, int, const glm::vec4&);
    using getPixel_t = glm::vec4(Bitmap::*)(int, int) const;
    using setPixel3D_t = void(Bitmap::*)(int, int, int, const glm::vec4&);
    using getPixel3D_t = glm::vec4(Bitmap::*)(int, int, int) const;

    // These are assigned in initGetSetFuncs()
    setPixel_t   setPixelFunc = &Bitmap::setPixelUnsignedByte;
    getPixel_t   getPixelFunc = &Bitmap::getPixelUnsignedByte;
    setPixel3D_t setPixel3DFunc = &Bitmap::setPixel3DUnsignedByte;
    getPixel3D_t getPixel3DFunc = &Bitmap::getPixel3DUnsignedByte;

    // Called from constructors to set up function pointers based on 'fmt_'
    void initGetSetFuncs()
    {
        switch (fmt_)
        {
        case eBitmapFormat_UnsignedByte:
            setPixelFunc = &Bitmap::setPixelUnsignedByte;
            getPixelFunc = &Bitmap::getPixelUnsignedByte;
            setPixel3DFunc = &Bitmap::setPixel3DUnsignedByte;
            getPixel3DFunc = &Bitmap::getPixel3DUnsignedByte;
            break;
        case eBitmapFormat_Float:
            setPixelFunc = &Bitmap::setPixelFloat;
            getPixelFunc = &Bitmap::getPixelFloat;
            setPixel3DFunc = &Bitmap::setPixel3DFloat;
            getPixel3DFunc = &Bitmap::getPixel3DFloat;
            break;
        }
    }

    // ====================================================
    // 2D set/get for float
    // ====================================================
    void setPixelFloat(int x, int y, const glm::vec4& c)
    {
        const int ofs = comp_ * (y * w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if (comp_ > 0) data[ofs + 0] = c.x;
        if (comp_ > 1) data[ofs + 1] = c.y;
        if (comp_ > 2) data[ofs + 2] = c.z;
        if (comp_ > 3) data[ofs + 3] = c.w;
    }
    glm::vec4 getPixelFloat(int x, int y) const
    {
        const int ofs = comp_ * (y * w_ + x);
        const float* data = reinterpret_cast<const float*>(data_.data());
        glm::vec4 color(0.0f);
        if (comp_ > 0) color.x = data[ofs + 0];
        if (comp_ > 1) color.y = data[ofs + 1];
        if (comp_ > 2) color.z = data[ofs + 2];
        if (comp_ > 3) color.w = data[ofs + 3];
        return color;
    }

    // ====================================================
    // 2D set/get for unsigned byte
    // ====================================================
    void setPixelUnsignedByte(int x, int y, const glm::vec4& c)
    {
        const int ofs = comp_ * (y * w_ + x);
        if (comp_ > 0) data_[ofs + 0] = uint8_t(glm::clamp(c.x, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 1) data_[ofs + 1] = uint8_t(glm::clamp(c.y, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 2) data_[ofs + 2] = uint8_t(glm::clamp(c.z, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 3) data_[ofs + 3] = uint8_t(glm::clamp(c.w, 0.0f, 1.0f) * 255.0f);
    }
    glm::vec4 getPixelUnsignedByte(int x, int y) const
    {
        const int ofs = comp_ * (y * w_ + x);
        glm::vec4 color(0.0f);
        if (comp_ > 0) color.x = float(data_[ofs + 0]) / 255.0f;
        if (comp_ > 1) color.y = float(data_[ofs + 1]) / 255.0f;
        if (comp_ > 2) color.z = float(data_[ofs + 2]) / 255.0f;
        if (comp_ > 3) color.w = float(data_[ofs + 3]) / 255.0f;
        return color;
    }

    // ====================================================
    // 3D set/get: same logic, but we index an additional 'z' or 'slice'
    // ====================================================
    void setPixel3DFloat(int x, int y, int z, const glm::vec4& c)
    {
        const int ofs = comp_ * (z * (w_ * h_) + (y * w_) + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if (comp_ > 0) data[ofs + 0] = c.x;
        if (comp_ > 1) data[ofs + 1] = c.y;
        if (comp_ > 2) data[ofs + 2] = c.z;
        if (comp_ > 3) data[ofs + 3] = c.w;
    }
    glm::vec4 getPixel3DFloat(int x, int y, int z) const
    {
        const int ofs = comp_ * (z * (w_ * h_) + (y * w_) + x);
        const float* data = reinterpret_cast<const float*>(data_.data());
        glm::vec4 color(0.0f);
        if (comp_ > 0) color.x = data[ofs + 0];
        if (comp_ > 1) color.y = data[ofs + 1];
        if (comp_ > 2) color.z = data[ofs + 2];
        if (comp_ > 3) color.w = data[ofs + 3];
        return color;
    }

    void setPixel3DUnsignedByte(int x, int y, int z, const glm::vec4& c)
    {
        const int ofs = comp_ * (z * (w_ * h_) + (y * w_) + x);
        if (comp_ > 0) data_[ofs + 0] = uint8_t(glm::clamp(c.x, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 1) data_[ofs + 1] = uint8_t(glm::clamp(c.y, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 2) data_[ofs + 2] = uint8_t(glm::clamp(c.z, 0.0f, 1.0f) * 255.0f);
        if (comp_ > 3) data_[ofs + 3] = uint8_t(glm::clamp(c.w, 0.0f, 1.0f) * 255.0f);
    }
    glm::vec4 getPixel3DUnsignedByte(int x, int y, int z) const
    {
        const int ofs = comp_ * (z * (w_ * h_) + (y * w_) + x);
        glm::vec4 color(0.0f);
        if (comp_ > 0) color.x = float(data_[ofs + 0]) / 255.0f;
        if (comp_ > 1) color.y = float(data_[ofs + 1]) / 255.0f;
        if (comp_ > 2) color.z = float(data_[ofs + 2]) / 255.0f;
        if (comp_ > 3) color.w = float(data_[ofs + 3]) / 255.0f;
        return color;
    }
};