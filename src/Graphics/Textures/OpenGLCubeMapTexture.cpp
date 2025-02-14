#include "OpenGLCubeMapTexture.h"
#include "TextureUtils.h"
#include "Bitmap.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <cmath>
#include <stdexcept>

namespace graphics {

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::array<std::filesystem::path, 6>& faces,
        const TextureConfig& config)
    {
        // 1) Load first face to get width_/height_
        Bitmap face0;
        face0.LoadFromFile(faces[0].string(), /*flipY=*/false, /*force4Ch=*/true);
        width_ = face0.width();
        height_ = face0.height();

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture_id_);
        if (!texture_id_) {
            throw std::runtime_error("OpenGLCubeMapTexture: glCreateTextures failed!");
        }

        GLenum finalFmt = ResolveInternalFormat(config, face0.components());
        int levels = config.generate_mips_
            ? (int)std::floor(std::log2(std::max(width_, height_))) + 1
            : 1;

        glTextureStorage2D(texture_id_, levels, finalFmt, width_, height_);

        // Upload each face
        for (int i = 0; i < 6; ++i) {
            Bitmap face;
            face.LoadFromFile(faces[i].string(), /*flipY=*/false, /*force4Ch=*/true);
            GLenum type = DataTypeForHDR(face.IsHDR());
            GLenum format = GL_RGBA;

            if (!face.data().empty()) {
                glTextureSubImage3D(texture_id_,
                    0,
                    0, 0, i,
                    width_, height_, 1,
                    format, type,
                    face.IsHDR() ? (const void*)face.GetDataFloat()
                    : (const void*)face.GetDataU8());
            }
            else {
                // Optionally throw if empty
            }
        }

        // Set texture params
        SetupTextureParameters(texture_id_, config, /*isCubeMap=*/true);

        // Make bindless if requested
        MakeBindlessIfNeeded(config.use_bindless_);
    }

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::vector<std::array<std::filesystem::path, 6>>& mip_faces,
        const TextureConfig& config)
    {
        if (mip_faces.empty()) {
            throw std::runtime_error("OpenGLCubeMapTexture: no mip faces!");
        }
        // 1) Load first face of first level
        Bitmap face0;
        face0.LoadFromFile(mip_faces[0][0].string(), false, true);
        width_ = face0.width();
        height_ = face0.height();

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture_id_);
        if (!texture_id_) {
            throw std::runtime_error("OpenGLCubeMapTexture: glCreateTextures failed!");
        }

        int mipLevels = (int)mip_faces.size();
        GLenum finalFmt = ResolveInternalFormat(config, face0.components());

        glTextureStorage2D(texture_id_, mipLevels, finalFmt, width_, height_);

        for (int level = 0; level < mipLevels; ++level) {
            for (int i = 0; i < 6; ++i) {
                Bitmap face;
                face.LoadFromFile(mip_faces[level][i].string(), false, true);
                GLenum type = DataTypeForHDR(face.IsHDR());
                GLenum format = GL_RGBA;

                glTextureSubImage3D(texture_id_,
                    level,
                    0, 0, i,
                    width_, height_, 1,
                    format, type,
                    face.IsHDR() ? (const void*)face.GetDataFloat()
                    : (const void*)face.GetDataU8());
            }
        }

        // Mipmap generation if desired. Actually we already allocated all levels, so might not need it.
        if (config.generate_mips_) {
            glGenerateTextureMipmap(texture_id_);
        }

        // Param + bindless
        SetupTextureParameters(texture_id_, config, /*isCubeMap=*/true);
        MakeBindlessIfNeeded(config.use_bindless_);
    }

} // namespace graphics