#include "Shader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

Shader::Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderPaths, std::filesystem::path binaryPath)
    : BaseShader(std::move(binaryPath)), m_ShaderPaths(shaderPaths) {
    LoadShader();
}

void Shader::ReloadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }
    m_UniformLocationCache.clear();
    LoadShader(true);
}

void Shader::LoadShader(bool bReload) {
    if (!bReload && !m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded shader binary from '{}'.", m_BinaryPath.string());
        return;
    }

    std::vector<GLuint> shaders;
    for (const auto& [shaderType, shaderPath] : m_ShaderPaths) {
        std::string source = ReadFile(shaderPath);
        std::unordered_set<std::string> includedFiles;
        source = ResolveIncludes(source, shaderPath.parent_path(), includedFiles);
        shaders.push_back(CompileShader(shaderType, source));
    }

    m_RendererID = LinkProgram(shaders);

    if (!m_BinaryPath.empty()) {
        SaveBinary();
    }
}

void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) {
    GLuint blockIndex = glGetUniformBlockIndex(m_RendererID, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_RendererID, blockIndex, bindingPoint);
    }
    else {
        Logger::GetLogger()->warn("Uniform block '{}' not found in shader program ID {}.", blockName, m_RendererID);
    }
}

void Shader::BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint) {
    GLuint blockIndex = glGetProgramResourceIndex(m_RendererID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX) {
        glShaderStorageBlockBinding(m_RendererID, blockIndex, bindingPoint);
    }
    else {
        Logger::GetLogger()->warn("Shader storage block '{}' not found in shader program ID {}.", blockName, m_RendererID);
    }
}