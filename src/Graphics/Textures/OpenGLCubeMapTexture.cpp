#include "OpenGLCubeMapTexture.h"
#include "TextureData.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <vector>
#include <cmath>
#include <filesystem>

namespace graphics {

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
        const TextureConfig& config) {
        InitializeCubeMap(faces, config);
    }

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces,
        const TextureConfig& config) {
        InitializeCubeMapMip(mip_faces, config);
    }

    void OpenGLCubeMapTexture::InitializeCubeMap(const std::array<std::filesystem::path, 6>& faces,
        const TextureConfig& config) {
        // Load first face to determine dimensions.
        TextureData faceData;
        if (!faceData.LoadFromFile(faces[0].string(), false, true, config.is_hdr_)) {
            throw std::runtime_error("OpenGLCubeMapTexture: Failed to load face: " + faces[0].string());
        }
        width_ = static_cast<uint32_t>(faceData.GetWidth());
        height_ = static_cast<uint32_t>(faceData.GetHeight());
        GLenum internal_format = config.internal_format_;

        GLCall(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture_id_));
        if (texture_id_ == 0) {
            throw std::runtime_error("OpenGLCubeMapTexture: Failed to create cube map texture.");
        }
        int levels = config.generate_mips_
            ? static_cast<int>(std::floor(std::log2(std::max(width_, height_)))) + 1
            : 1;
        GLCall(glTextureStorage2D(texture_id_, levels, internal_format, width_, height_));

        // Load each face.
        for (size_t i = 0; i < 6; ++i) {
            TextureData data;
            if (!data.LoadFromFile(faces[i].string(), false, true, config.is_hdr_)) {
                throw std::runtime_error("OpenGLCubeMapTexture: Failed to load face: " + faces[i].string());
            }
            GLenum dataType = config.is_hdr_ ? GL_FLOAT : GL_UNSIGNED_BYTE;
            GLenum uploadFormat = GL_RGBA; // assuming data is RGBA
            const void* pixels = config.is_hdr_
                ? static_cast<const void*>(data.GetDataFloat())
                : static_cast<const void*>(data.GetDataU8());
            if (!pixels) {
                throw std::runtime_error("OpenGLCubeMapTexture: No pixel data for face: " + faces[i].string());
            }
            GLCall(glTextureSubImage3D(texture_id_, 0, 0, 0, i, width_, height_, 1, uploadFormat, dataType, pixels));
        }
        if (config.generate_mips_) {
            GLCall(glGenerateTextureMipmap(texture_id_));
        }
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MIN_FILTER, config.min_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_S, config.wrap_s_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_T, config.wrap_t_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_R, config.wrap_r_));

        if (config.use_bindless_ && GLAD_GL_ARB_bindless_texture) {
            bindless_handle_ = glGetTextureHandleARB(texture_id_);
            if (bindless_handle_) {
                glMakeTextureHandleResidentARB(bindless_handle_);
                is_bindless_ = true;
            }
        }
    }

    void OpenGLCubeMapTexture::InitializeCubeMapMip(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces,
        const TextureConfig& config) {
        if (mip_faces.empty()) {
            throw std::runtime_error("OpenGLCubeMapTexture: No mip faces provided.");
        }
        // Use first face of first mip level for dimensions.
        TextureData faceData;
        if (!faceData.LoadFromFile(mip_faces[0][0].string(), false, true, config.is_hdr_)) {
            throw std::runtime_error("OpenGLCubeMapTexture: Failed to load face: " + mip_faces[0][0].string());
        }
        width_ = static_cast<uint32_t>(faceData.GetWidth());
        height_ = static_cast<uint32_t>(faceData.GetHeight());
        GLenum internal_format = config.internal_format_;
        int mipLevels = static_cast<int>(mip_faces.size());

        GLCall(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture_id_));
        if (texture_id_ == 0) {
            throw std::runtime_error("OpenGLCubeMapTexture: Failed to create cube map texture.");
        }
        GLCall(glTextureStorage2D(texture_id_, mipLevels, internal_format, width_, height_));

        for (int level = 0; level < mipLevels; ++level) {
            const auto& faces = mip_faces[level];
            for (size_t i = 0; i < 6; ++i) {
                TextureData data;
                if (!data.LoadFromFile(faces[i].string(), false, true, config.is_hdr_)) {
                    throw std::runtime_error("OpenGLCubeMapTexture: Failed to load face at mip level " + std::to_string(level) +
                        ": " + faces[i].string());
                }
                GLenum dataType = config.is_hdr_ ? GL_FLOAT : GL_UNSIGNED_BYTE;
                GLenum uploadFormat = GL_RGBA;
                const void* pixels = config.is_hdr_
                    ? static_cast<const void*>(data.GetDataFloat())
                    : static_cast<const void*>(data.GetDataU8());
                if (!pixels) {
                    throw std::runtime_error("OpenGLCubeMapTexture: No pixel data for face at mip level " + std::to_string(level) +
                        ": " + faces[i].string());
                }
                GLCall(glTextureSubImage3D(texture_id_, level, 0, 0, i, width_, height_, 1, uploadFormat, dataType, pixels));
            }
        }
        if (config.generate_mips_) {
            GLCall(glGenerateTextureMipmap(texture_id_));
        }
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MIN_FILTER, config.min_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_S, config.wrap_s_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_T, config.wrap_t_));
        GLCall(glTextureParameteri(texture_id_, GL_TEXTURE_WRAP_R, config.wrap_r_));

        if (config.use_bindless_ && GLAD_GL_ARB_bindless_texture) {
            bindless_handle_ = glGetTextureHandleARB(texture_id_);
            if (bindless_handle_) {
                glMakeTextureHandleResidentARB(bindless_handle_);
                is_bindless_ = true;
            }
        }
    }

    OpenGLCubeMapTexture::~OpenGLCubeMapTexture() {
        if (is_bindless_ && bindless_handle_) {
            glMakeTextureHandleNonResidentARB(bindless_handle_);
        }
        if (texture_id_) {
            GLCall(glDeleteTextures(1, &texture_id_));
        }
    }

    void OpenGLCubeMapTexture::Bind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, texture_id_));
        }
    }

    void OpenGLCubeMapTexture::Unbind(uint32_t unit) const {
        if (!is_bindless_) {
            GLCall(glBindTextureUnit(unit, 0));
        }
    }

} // namespace graphics
