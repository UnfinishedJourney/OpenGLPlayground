#include "ComputeShader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

ComputeShader::ComputeShader(const std::filesystem::path& shaderPath, std::filesystem::path binaryPath)
    : BaseShader(std::move(binaryPath)), m_ShaderPath(shaderPath) {
    LoadShader();
}

void ComputeShader::ReloadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }
    m_UniformLocationCache.clear();
    LoadShader(true);
}

void ComputeShader::LoadShader(bool bReload) {
    if (!bReload && !m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded compute shader binary from '{}'.", m_BinaryPath.string());
        return;
    }

    // Read shader source
    std::string source = ReadFile(m_ShaderPath);
    std::unordered_set<std::string> includedFiles;
    source = ResolveIncludes(source, m_ShaderPath.parent_path(), includedFiles);

    // Compile shader
    GLuint shader = CompileShader(GL_COMPUTE_SHADER, source);

    // Link program
    m_RendererID = LinkProgram({ shader });

    // Save binary if binary path is provided
    if (!m_BinaryPath.empty()) {
        SaveBinary();
    }
}

void ComputeShader::Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
    glUseProgram(m_RendererID);
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    glUseProgram(0);
}