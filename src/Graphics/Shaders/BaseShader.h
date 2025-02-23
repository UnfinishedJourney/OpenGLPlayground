#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include "IShader.h"

namespace graphics {

    /**
     * @brief Base class for OpenGL shader programs.
     *
     * Implements common functionality such as shader compilation, linking,
     * uniform caching, file reading, include resolution, and binary caching.
     */
    class BaseShader : public IShader {
    public:
        /**
         * @brief Constructor.
         * @param binaryPath Optional path for saving/loading the binary.
         */
        explicit BaseShader(const std::filesystem::path& binaryPath = "");

        virtual ~BaseShader();

        // Non-copyable.
        BaseShader(const BaseShader&) = delete;
        BaseShader& operator=(const BaseShader&) = delete;

        // Movable.
        BaseShader(BaseShader&& other) noexcept;
        BaseShader& operator=(BaseShader&& other) noexcept;

        // IShader interface.
        void Bind() const override;
        void Unbind() const override;
        virtual void ReloadShader() override = 0;

        // Uniform setters.
        void SetUniform(std::string_view name, float value) const override;
        void SetUniform(std::string_view name, int value) const override;
        void SetUniform(std::string_view name, unsigned int value) const override;
        void SetUniform(std::string_view name, const glm::vec2& value) const override;
        void SetUniform(std::string_view name, const glm::vec3& value) const override;
        void SetUniform(std::string_view name, const glm::vec4& value) const override;
        void SetUniform(std::string_view name, const glm::mat3& value) const override;
        void SetUniform(std::string_view name, const glm::mat4& value) const override;

        /**
         * @brief Attempt to load the program from a binary file.
         * @return True on success, false otherwise.
         */
        bool LoadBinary();

    protected:
        GLuint m_RendererId_;  ///< OpenGL program handle.
        std::filesystem::path m_BinaryPath_;  ///< Path for loading/saving the binary.
        mutable std::unordered_map<std::string, GLint> m_UniformLocationCache_;  ///< Cache of uniform locations.

        // Helper functions.
        GLuint CompileShader(GLenum shaderType, const std::string& source) const;
        GLuint LinkProgram(const std::vector<GLuint>& shaders) const;
        std::string ReadFile(const std::filesystem::path& filepath) const;
        std::string ResolveIncludes(const std::string& source,
            const std::filesystem::path& directory,
            std::unordered_set<std::string>& includedFiles) const;
        void SaveBinary() const;

        GLint GetUniformLocation(std::string_view name) const;
    };

} // namespace graphics