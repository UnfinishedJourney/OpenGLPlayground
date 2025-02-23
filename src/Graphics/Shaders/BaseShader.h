#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include "IShader.h"

namespace graphics {

    /**
     * @brief Base class for OpenGL shader programs.
     *
     * Provides common functionality like shader compilation, linking, uniform caching,
     * file reading, include resolution, and binary caching.
     */
    class BaseShader : public IShader {
    public:
        explicit BaseShader(const std::filesystem::path& binaryPath = "");
        virtual ~BaseShader();

        BaseShader(const BaseShader&) = delete;
        BaseShader& operator=(const BaseShader&) = delete;
        BaseShader(BaseShader&& other) noexcept;
        BaseShader& operator=(BaseShader&& other) noexcept;

        void Bind() const override;
        void Unbind() const override;
        virtual void ReloadShader() override = 0;

        void SetUniform(std::string_view name, float value) const override;
        void SetUniform(std::string_view name, int value) const override;
        void SetUniform(std::string_view name, unsigned int value) const override;
        void SetUniform(std::string_view name, const glm::vec2& value) const override;
        void SetUniform(std::string_view name, const glm::vec3& value) const override;
        void SetUniform(std::string_view name, const glm::vec4& value) const override;
        void SetUniform(std::string_view name, const glm::mat3& value) const override;
        void SetUniform(std::string_view name, const glm::mat4& value) const override;

        bool LoadBinary();

    protected:
        GLuint rendererId_;  ///< OpenGL program handle.
        std::filesystem::path binaryPath_;  ///< Path for binary caching.
        mutable std::unordered_map<std::string, GLint> uniformLocationCache_;  ///< Uniform location cache.

        GLuint CompileShader(GLenum shaderType, const std::string& source) const;
        GLuint LinkProgram(const std::vector<GLuint>& shaders) const;
        std::string ReadFile(const std::filesystem::path& filepath) const;
        std::string ResolveIncludes(const std::string& source, const std::filesystem::path& directory,
            std::unordered_set<std::string>& includedFiles) const;
        void SaveBinary() const;
        GLint GetUniformLocation(std::string_view name) const;
    };

} // namespace graphics