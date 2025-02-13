#pragma once
#include <string>
#include <vector>

namespace graphics {

    /**
     * @brief Loads image data from disk (LDR or HDR).
     */
    class TextureData {
    public:
        bool LoadFromFile(const std::string& file_path,
            bool flip_y = true,
            bool force_4ch = true,
            bool is_hdr = false);

        const unsigned char* GetDataU8() const { return byte_data_.empty() ? nullptr : byte_data_.data(); }
        const float* GetDataFloat() const { return float_data_.empty() ? nullptr : float_data_.data(); }

        int GetWidth() const { return width_; }
        int GetHeight() const { return height_; }
        int GetChannels() const { return channels_; }
        bool IsHDR() const { return is_hdr_; }

    private:
        std::vector<unsigned char> byte_data_;
        std::vector<float> float_data_;
        int width_ = 0;
        int height_ = 0;
        int channels_ = 0;
        bool is_hdr_ = false;
    };

} // namespace graphics