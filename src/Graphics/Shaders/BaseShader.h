#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "ShaderDeleter.h"

/**
 * @brief Base class for OpenGL shader programs (vertex/fragment/geometry/tess/compute).
 *
 * Responsible for:
 *  - Creating & storing a GL program handle
 *  - Loading and saving program binaries if a path is provided
 *  - Setting common uniforms (floats, ints, vectors, matrices)
 *  - Recursively resolving #includes in GLSL sources
 */
class BaseShader {
public:
    /**
     * @param binaryPath Optional path for loading/saving the compiled program.
     */
    explicit BaseShader(const std::filesystem::path& binaryPath = "");
    virtual ~BaseShader() noexcept = default;

    // Non-copyable, move-constructible
    BaseShader(const BaseShader&) = delete;
    BaseShader& operator=(const BaseShader&) = delete;

    BaseShader(BaseShader&&) noexcept = default;
    BaseShader& operator=(BaseShader&&) noexcept = default;

    /**
     * @brief Reload the shader from source or binary (user must override).
     */
    virtual void ReloadShader() = 0;

    /**
     * @brief Binds this shader program (glUseProgram).
     */
    void Bind() const noexcept;

    /**
     * @brief Unbinds any current shader program (glUseProgram(0)).
     */
    void Unbind() const noexcept;

    // ----------------------------------------------------------------------
    // Uniform setters
    // ----------------------------------------------------------------------
    void SetUniform(std::string_view name, float value) const;
    void SetUniform(std::string_view name, int value) const;
    void SetUniform(std::string_view name, unsigned int value) const;
    void SetUniform(std::string_view name, const glm::vec2& value) const;
    void SetUniform(std::string_view name, const glm::vec3& value) const;
    void SetUniform(std::string_view name, const glm::vec4& value) const;
    void SetUniform(std::string_view name, const glm::mat3& value) const;
    void SetUniform(std::string_view name, const glm::mat4& value) const;

    /**
     * @brief Attempt to load the program from the binary file specified by m_BinaryPath (if not empty).
     * @return True on success, false otherwise.
     */
    bool LoadBinary();

protected:
    /**
     * @brief A unique handle for the GL program, with a custom deleter that calls glDeleteProgram().
     */
    std::unique_ptr<GLuint, ShaderDeleter> m_RendererIDPtr;

    /**
     * @brief Path to load/save program binaries.
     */
    std::filesystem::path m_BinaryPath;

    /**
     * @brief Cache of uniform locations for quick lookups.
     */
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    // ----------------------------------------------------------------------
    // Protected utility methods for derived classes.
    // ----------------------------------------------------------------------

    /**
     * @brief Compile a single shader (GL_VERTEX_SHADER, etc.) from source.
     * @throws std::runtime_error on compilation failure.
     * @return The compiled shader handle.
     */
    GLuint CompileShader(GLenum shaderType, const std::string& source) const;

    /**
     * @brief Link multiple compiled shader objects into a single program.
     * @throws std::runtime_error on linking failure.
     * @return The linked program handle (GLuint).
     */
    GLuint LinkProgram(const std::vector<GLuint>& shaders) const;

    /**
     * @brief Read a text file from disk into a std::string.
     * @throws std::runtime_error if file can't be opened or doesn't exist.
     */
    std::string ReadFile(const std::filesystem::path& filepath) const;

    /**
     * @brief Recursively resolve #include directives in the shader source code.
     * @throws std::runtime_error on invalid #include syntax or missing file.
     * @param source       The current shader source text.
     * @param directory    The base directory to search for includes.
     * @param includedFiles A set to track files already included (avoid duplicates).
     * @return The fully resolved shader source with includes expanded.
     */
    std::string ResolveIncludes(const std::string& source,
        const std::filesystem::path& directory,
        std::unordered_set<std::string>& includedFiles) const;

    /**
     * @brief Save the current program object to a binary file (m_BinaryPath).
     */
    void SaveBinary() const;

    /**
     * @brief Retrieve uniform location, caching results for efficiency.
     * @param name The uniform name (in the GLSL program).
     * @return The uniform location or -1 if not found.
     */
    GLint GetUniformLocation(std::string_view name) const;
};