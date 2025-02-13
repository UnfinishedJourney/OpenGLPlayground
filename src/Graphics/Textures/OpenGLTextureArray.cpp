#include "OpenGLTextureArray.h"
#include "TextureData.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace graphics {

    OpenGLTextureArray::OpenGLTextureArray(const std::vector<std::string>& file_paths,
        const TextureConfig& config,
        uint32_t total_frames,
        uint32_t grid_x,
        uint32_t grid_y)
    {
        if (file_paths.empty()) {
            throw std::runtime_error("OpenGLTextureArray: No input files provided.");
        }
        if (file_paths.size() > 1) {
            throw std::runtime_error("OpenGLTextureArray: Multiple files not supported in this scenario.");
        }

        // Load the full sprite sheet from the first file.
        TextureData data;
        if (!data.LoadFromFile(file_paths[0], /*flipY=*/false, /*force4Ch=*/true, /*isHDR=*/false)) {
            throw std::runtime_error("OpenGLTextureArray: Failed to load texture from " + file_paths[0]);
        }
        width_ = static_cast<uint32_t>(data.GetWidth());
        height_ = static_cast<uint32_t>(data.GetHeight());
        int channels = data.GetChannels();

        // Compute frame dimensions.
        uint32_t frame_width = width_ / grid_x;
        uint32_t frame_height = height_ / grid_y;

        GLCall(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_id_));
        if (!texture_id_) {
            throw std::runtime_error("OpenGLTextureArray: Failed to create texture array object.");
        }

        int levels = config.generate_mips_
            ? static_cast<int>(std::floor(std::log2(std::max(frame_width, frame_height)))) + 1
            : 1;
        GLCall(glTextureStorage3D(texture_id_, levels, config.internal_format_, frame_width, frame_height, total_frames));

        // Set unpack alignment.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        const unsigned char* full_image_data = data.GetDataU8();
        if (!full_image_data) {
            throw std::runtime_error("OpenGLTextureArray: No 8-bit data found (expected LDR).");
        }
        size_t frame_size = static_cast<size_t>(frame_width) * frame_height * channels;
        std::vector<unsigned char> frame_data(frame_size);
        uint32_t frame_index = 0;
        for (uint32_t gy = 0; gy < grid_y && frame_index < total_frames; gy++) {
            for (uint32_t gx = 0; gx < grid_x && frame_index < total_frames; gx++) {
                uint32_t src_x = gx * frame_width;
                uint32_t src_y = gy * frame_height;
                for (uint32_t row = 0; row < frame_height; row++) {
                    size_t src_offset = ((src_y + row) * static_cast<size_t>(width_) + src_x) * channels;
                    size_t dst_offset = row * static_cast<size_t>(frame_width) * channels;
                    std::memcpy(&frame_data[dst_offset], &full_image_data[src_offset], frame_width * channels);
                }
                GLCall(glTextureSubImage3D(texture_id_,
                    0, // mip level
                    0, // x offset
                    0, // y offset
                    frame_index, // layer index
                    frame_width,
                    frame_height,
                    1, // one layer
                    GL_RGBA, // forcing 4 channels
                    GL_UNSIGNED_BYTE,
                    frame_data.data()));
                frame_index++;
            }
        }
        Logger::GetLogger()->info("OpenGLTextureArray: Loaded {} frames (expected {}).", frame_index, total_frames);
        if (config.generate_mips_) {
            GLCall(glGenerateTextureMipmap(texture_id_));
        }
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MIN_FILTER, config.min_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_S, config.wrap_s_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_T, config.wrap_t_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_R, config.wrap_r_));

        if (config.use_anisotropy_) {
            GLfloat max_aniso = 0.f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
            GLCall(glTextureParameterf(texture_id_, GL_TEXTURE_MAX_ANISOTROPY, max_aniso));
        }
        if (config.use_bindless_ && GLAD_GL_ARB_bindless_texture) {
            bindless_handle_ = glGetTextureHandleARB(texture_id_);
            if (bindless_handle_) {
                glMakeTextureHandleResidentARB(bindless_handle_);
                is_bindless_ = true;
            }
        }
    }

    OpenGLTextureArray::~OpenGLTextureArray() {
        if (is_bindless_ && bindless_handle_) {
            glMakeTextureHandleNonResidentARB(bindless_handle_);
        }
        if (texture_id_) {
            GLCall(glDeleteTextures(1, &texture_id_));
        }
    }

    void OpenGLTextureArray::Bind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, texture_id_));
        }
    }

    void OpenGLTextureArray::Unbind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace graphics