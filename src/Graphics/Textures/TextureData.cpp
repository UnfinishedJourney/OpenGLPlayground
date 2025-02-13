#include "TextureData.h"
#include <stb_image.h>
#include <cstring>

namespace graphics {

    bool TextureData::LoadFromFile(const std::string& file_path,
        bool flip_y,
        bool force_4ch,
        bool is_hdr) {
        is_hdr_ = is_hdr;
        stbi_set_flip_vertically_on_load(flip_y ? 1 : 0);
        if (is_hdr_) {
            float* data = force_4ch
                ? stbi_loadf(file_path.c_str(), &width_, &height_, &channels_, 4)
                : stbi_loadf(file_path.c_str(), &width_, &height_, &channels_, 0);
            if (!data) return false;
            if (force_4ch) { channels_ = 4; }
            size_t size = static_cast<size_t>(width_) * height_ * channels_;
            float_data_.resize(size);
            std::memcpy(float_data_.data(), data, size * sizeof(float));
            stbi_image_free(data);
        }
        else {
            unsigned char* data = stbi_load(file_path.c_str(), &width_, &height_, &channels_, force_4ch ? 4 : 0);
            if (!data) return false;
            if (force_4ch) { channels_ = 4; }
            size_t size = static_cast<size_t>(width_) * height_ * channels_;
            byte_data_.resize(size);
            std::memcpy(byte_data_.data(), data, size);
            stbi_image_free(data);
        }
        stbi_set_flip_vertically_on_load(0);
        return true;
    }

} // namespace graphics
