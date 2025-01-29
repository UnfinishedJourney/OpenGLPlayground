#pragma once

#include "BaseShader.h"
#include <unordered_map>
#include <filesystem>

/**
 * @brief A standard shader class supporting vertex/fragment/geometry/etc. stages.
 *
 * Use the constructor to supply a map of shader stage -> file path.
 * For example:
 * {
 *   { GL_VERTEX_SHADER,   "myVert.glsl" },
 *   { GL_FRAGMENT_SHADER, "myFrag.glsl" }
 * }
 */
class Shader : public BaseShader {
public:
    /**
     * @param shaderStages A map from GLenum (e.g., GL_VERTEX_SHADER) to a file path for that shader source.
     * @param binaryPath   (Optional) path for saving/loading a program binary.
     */
    Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
        const std::filesystem::path& binaryPath = "");

    /**
     * @brief Reload from source or binary, clearing the old program.
     */
    void ReloadShader() override;

    /**
     * @brief Binds a uniform block name to a specific binding point (e.g., for UBOs).
     */
    void BindUniformBlock(const std::string& blockName, GLuint bindingPoint);

    /**
     * @brief Binds a shader storage block name to a specific binding point (e.g., for SSBOs).
     */
    void BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint);

private:
    void LoadShaders(bool reload = false);

    std::unordered_map<GLenum, std::filesystem::path> m_ShaderStages;
};