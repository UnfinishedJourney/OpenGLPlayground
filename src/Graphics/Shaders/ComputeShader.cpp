#include "ComputeShader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

ComputeShader::ComputeShader(std::filesystem::path sourcePath, std::filesystem::path binaryPath)
    : BaseShader(std::move(sourcePath), std::move(binaryPath)) {
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
    std::string source = ReadFile(m_SourcePath);
    std::unordered_set<std::string> includedFiles;
    source = ResolveIncludes(source, m_SourcePath.parent_path(), includedFiles);

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