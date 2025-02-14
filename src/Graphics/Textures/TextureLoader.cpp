#include "TextureLoader.h"
#include "Bitmap.h"
#include "OpenGLTexture.h"
#include "OpenGLCubeMapTexture.h"
#include "OpenGLTextureArray.h"
#include "TextureUtils.h"
#include "Utilities/Logger.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include <stdexcept>
#include <filesystem>
#include <array>
#include <glm/glm.hpp>

namespace graphics {

    std::shared_ptr<ITexture> TextureLoader::Load2DTexture(const std::string& filePath, const TextureConfig& config) {
        Bitmap bitmap;
        try {
            if (!bitmap.LoadFromFile(filePath, true, true) || bitmap.data().empty()) {
                Logger::GetLogger()->error("Load2DTexture: Failed to load texture from '{}'.", filePath);
                throw std::runtime_error("2D texture load failed.");
            }
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Load2DTexture: Exception loading '{}': {}", filePath, e.what());
            throw;
        }
        try {
            auto texture = std::make_shared<OpenGLTexture>(bitmap, config);
            Logger::GetLogger()->info("Load2DTexture: Loaded texture from '{}'.", filePath);
            return texture;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Load2DTexture: Exception creating texture from '{}': {}", filePath, e.what());
            throw;
        }
    }

    std::shared_ptr<ITexture> TextureLoader::LoadCubeMapTexture(const std::array<std::string, 6>& facePathsStr, const TextureConfig& config) {
        std::array<std::filesystem::path, 6> facePaths;
        for (size_t i = 0; i < facePathsStr.size(); ++i) {
            facePaths[i] = std::filesystem::path(facePathsStr[i]);
        }
        try {
            auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(facePaths, config);
            Logger::GetLogger()->info("LoadCubeMapTexture: Loaded cubemap texture.");
            return cubeMap;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("LoadCubeMapTexture: Exception: {}", e.what());
            throw;
        }
    }

    std::shared_ptr<ITexture> TextureLoader::LoadTextureArray(const std::string& filePath, const TextureConfig& config,
        uint32_t totalFrames, uint32_t gridX, uint32_t gridY) {
        try {
            auto textureArray = std::make_shared<OpenGLTextureArray>(std::vector<std::string>{filePath}, config, totalFrames, gridX, gridY);
            Logger::GetLogger()->info("LoadTextureArray: Loaded texture array from '{}'.", filePath);
            return textureArray;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("LoadTextureArray: Exception loading '{}': {}", filePath, e.what());
            throw;
        }
    }

    std::shared_ptr<ITexture> TextureLoader::CreateBRDFTexture(int width, int height, unsigned int numSamples) {
        auto& shaderManager = ShaderManager::GetInstance();
        auto computeShader = shaderManager.GetComputeShader("brdfCompute");
        if (!computeShader) {
            Logger::GetLogger()->error("CreateBRDFTexture: Compute shader 'brdfCompute' not found!");
            throw std::runtime_error("BRDF compute shader not found.");
        }
        auto ssbo = std::make_unique<ShaderStorageBuffer>(1, width * height * sizeof(glm::vec2), GL_DYNAMIC_COPY);
        computeShader->Bind();
        computeShader->SetUniform("NUM_SAMPLES", numSamples);
        GLuint groupX = (width + 15) / 16, groupY = (height + 15) / 16;
        computeShader->Dispatch(groupX, groupY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        computeShader->Unbind();

        GLuint brdfLUTID = 0;
        glGenTextures(1, &brdfLUTID);
        glBindTexture(GL_TEXTURE_2D, brdfLUTID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        ssbo->Bind();
        glm::vec2* ptr = static_cast<glm::vec2*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(glm::vec2), GL_MAP_READ_BIT));
        if (!ptr) {
            ssbo->Unbind();
            glDeleteTextures(1, &brdfLUTID);
            Logger::GetLogger()->error("CreateBRDFTexture: Failed to map SSBO.");
            throw std::runtime_error("SSBO mapping failed.");
        }
        glBindTexture(GL_TEXTURE_2D, brdfLUTID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RG, GL_FLOAT, ptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        ssbo->Unbind();

        class ComputedTexture : public ITexture {
        public:
            ComputedTexture(GLuint id, int w, int h) : texture_id_(id), width_(w), height_(h) {}
            ~ComputedTexture() override { if (texture_id_) glDeleteTextures(1, &texture_id_); }
            void Bind(uint32_t unit) const override { glBindTextureUnit(unit, texture_id_); }
            void Unbind(uint32_t unit) const override { glBindTextureUnit(unit, 0); }
            uint32_t GetWidth() const override { return width_; }
            uint32_t GetHeight() const override { return height_; }
            uint64_t GetBindlessHandle() const override { return 0; }
            bool IsBindless() const override { return false; }
        private:
            GLuint texture_id_ = 0;
            int width_ = 0, height_ = 0;
        };

        Logger::GetLogger()->info("CreateBRDFTexture: Created BRDF LUT texture (ID={}).", brdfLUTID);
        return std::make_shared<ComputedTexture>(brdfLUTID, width, height);
    }

} // namespace graphics