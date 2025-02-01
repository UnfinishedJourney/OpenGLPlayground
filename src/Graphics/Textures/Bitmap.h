#pragma once

#include <cstring>      // for memcpy
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

// Bitmap type: 2D image or Cubemap (6 faces stored consecutively).
enum eBitmapType
{
    eBitmapType_2D,
    eBitmapType_Cube
};

// Bitmap format: unsigned byte (0–255) or float (HDR).
enum eBitmapFormat
{
    eBitmapFormat_UnsignedByte,
    eBitmapFormat_Float,
};

/// A simple Bitmap class to hold image data with R/RG/RGB/RGBA channels.
/// This implementation supports both 2D images and cubemaps.
struct Bitmap
{
    // Constructors
    Bitmap() = default;
    Bitmap(int w, int h, int comp, eBitmapFormat fmt)
        : w_(w)
        , h_(h)
        , comp_(comp)
        , fmt_(fmt)
        , data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    // For 3D images (e.g. a cubemap with multiple faces)
    Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt)
        : w_(w)
        , h_(h)
        , d_(d)
        , comp_(comp)
        , fmt_(fmt)
        , data_(w* h* d* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    // Construct from existing data pointer.
    Bitmap(int w, int h, int comp, eBitmapFormat fmt, const void* ptr)
        : w_(w)
        , h_(h)
        , comp_(comp)
        , fmt_(fmt)
        , data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
        std::memcpy(data_.data(), ptr, data_.size());
    }

    // Image dimensions and parameters.
    int w_ = 0;         // width in pixels
    int h_ = 0;         // height in pixels
    int d_ = 1;         // depth (number of layers/faces), e.g. 6 for a cubemap
    int comp_ = 3;      // number of channels per pixel (e.g. 3 for RGB, 4 for RGBA)
    eBitmapFormat fmt_ = eBitmapFormat_UnsignedByte;
    eBitmapType type_ = eBitmapType_2D; // default to a 2D image
    std::vector<uint8_t> data_;          // pixel data stored consecutively

    // Returns the number of bytes per component based on format.
    static int getBytesPerComponent(eBitmapFormat fmt)
    {
        if (fmt == eBitmapFormat_UnsignedByte)
            return 1;
        if (fmt == eBitmapFormat_Float)
            return 4;
        return 0;
    }

    // --- 2D Pixel Access ---
    // Set pixel color at (x, y) (for 2D images).
    void setPixel(int x, int y, const glm::vec4& c)
    {
        (this->*setPixelFunc)(x, y, c);
    }
    // Get pixel color at (x, y) (for 2D images).
    glm::vec4 getPixel(int x, int y) const
    {
        return (this->*getPixelFunc)(x, y);
    }

    // --- Cubemap Pixel Access ---
    // If the bitmap is a cubemap (type_ == eBitmapType_Cube), these functions let you
    // set or get a pixel on a given face.
    void setPixel(int x, int y, int face, const glm::vec4& c)
    {
        if (type_ != eBitmapType_Cube)
        {
            // Fall back to the 2D version if not a cubemap.
            setPixel(x, y, c);
            return;
        }
        // Each face occupies w_ * h_ * comp_ components.
        int facePixelCount = w_ * h_ * comp_;
        int offset = face * facePixelCount + comp_ * (y * w_ + x);
        if (fmt_ == eBitmapFormat_Float)
        {
            float* data = reinterpret_cast<float*>(data_.data());
            if (comp_ > 0)
                data[offset + 0] = c.x;
            if (comp_ > 1)
                data[offset + 1] = c.y;
            if (comp_ > 2)
                data[offset + 2] = c.z;
            if (comp_ > 3)
                data[offset + 3] = c.w;
        }
        else if (fmt_ == eBitmapFormat_UnsignedByte)
        {
            if (comp_ > 0)
                data_[offset + 0] = uint8_t(c.x * 255.0f);
            if (comp_ > 1)
                data_[offset + 1] = uint8_t(c.y * 255.0f);
            if (comp_ > 2)
                data_[offset + 2] = uint8_t(c.z * 255.0f);
            if (comp_ > 3)
                data_[offset + 3] = uint8_t(c.w * 255.0f);
        }
    }

    glm::vec4 getPixel(int x, int y, int face) const
    {
        if (type_ != eBitmapType_Cube)
        {
            return getPixel(x, y);
        }
        int facePixelCount = w_ * h_ * comp_;
        int offset = face * facePixelCount + comp_ * (y * w_ + x);
        if (fmt_ == eBitmapFormat_Float)
        {
            const float* data = reinterpret_cast<const float*>(data_.data());
            return glm::vec4(
                comp_ > 0 ? data[offset + 0] : 0.0f,
                comp_ > 1 ? data[offset + 1] : 0.0f,
                comp_ > 2 ? data[offset + 2] : 0.0f,
                comp_ > 3 ? data[offset + 3] : 0.0f);
        }
        else if (fmt_ == eBitmapFormat_UnsignedByte)
        {
            return glm::vec4(
                comp_ > 0 ? float(data_[offset + 0]) / 255.0f : 0.0f,
                comp_ > 1 ? float(data_[offset + 1]) / 255.0f : 0.0f,
                comp_ > 2 ? float(data_[offset + 2]) / 255.0f : 0.0f,
                comp_ > 3 ? float(data_[offset + 3]) / 255.0f : 0.0f);
        }
        return glm::vec4();
    }

private:
    // Function pointer types for pixel access for 2D images.
    using setPixel_t = void(Bitmap::*)(int, int, const glm::vec4&);
    using getPixel_t = glm::vec4(Bitmap::*)(int, int) const;

    // By default, these pointers are set based on the image format.
    setPixel_t setPixelFunc = &Bitmap::setPixelUnsignedByte;
    getPixel_t getPixelFunc = &Bitmap::getPixelUnsignedByte;

    // Initialize the get/set function pointers based on the bitmap's format.
    void initGetSetFuncs()
    {
        switch (fmt_)
        {
        case eBitmapFormat_UnsignedByte:
            setPixelFunc = &Bitmap::setPixelUnsignedByte;
            getPixelFunc = &Bitmap::getPixelUnsignedByte;
            break;
        case eBitmapFormat_Float:
            setPixelFunc = &Bitmap::setPixelFloat;
            getPixelFunc = &Bitmap::getPixelFloat;
            break;
        default:
            // Default fallback.
            setPixelFunc = &Bitmap::setPixelUnsignedByte;
            getPixelFunc = &Bitmap::getPixelUnsignedByte;
            break;
        }
    }

    // --- 2D Pixel Access (format-specific implementations) ---
    // For floating-point data.
    void setPixelFloat(int x, int y, const glm::vec4& c)
    {
        int ofs = comp_ * (y * w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if (comp_ > 0)
            data[ofs + 0] = c.x;
        if (comp_ > 1)
            data[ofs + 1] = c.y;
        if (comp_ > 2)
            data[ofs + 2] = c.z;
        if (comp_ > 3)
            data[ofs + 3] = c.w;
    }
    glm::vec4 getPixelFloat(int x, int y) const
    {
        int ofs = comp_ * (y * w_ + x);
        const float* data = reinterpret_cast<const float*>(data_.data());
        return glm::vec4(
            comp_ > 0 ? data[ofs + 0] : 0.0f,
            comp_ > 1 ? data[ofs + 1] : 0.0f,
            comp_ > 2 ? data[ofs + 2] : 0.0f,
            comp_ > 3 ? data[ofs + 3] : 0.0f);
    }
    // For unsigned byte data.
    void setPixelUnsignedByte(int x, int y, const glm::vec4& c)
    {
        int ofs = comp_ * (y * w_ + x);
        if (comp_ > 0)
            data_[ofs + 0] = uint8_t(c.x * 255.0f);
        if (comp_ > 1)
            data_[ofs + 1] = uint8_t(c.y * 255.0f);
        if (comp_ > 2)
            data_[ofs + 2] = uint8_t(c.z * 255.0f);
        if (comp_ > 3)
            data_[ofs + 3] = uint8_t(c.w * 255.0f);
    }
    glm::vec4 getPixelUnsignedByte(int x, int y) const
    {
        int ofs = comp_ * (y * w_ + x);
        return glm::vec4(
            comp_ > 0 ? float(data_[ofs + 0]) / 255.0f : 0.0f,
            comp_ > 1 ? float(data_[ofs + 1]) / 255.0f : 0.0f,
            comp_ > 2 ? float(data_[ofs + 2]) / 255.0f : 0.0f,
            comp_ > 3 ? float(data_[ofs + 3]) / 255.0f : 0.0f);
    }
};
