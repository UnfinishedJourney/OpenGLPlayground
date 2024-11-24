#include "ComputeShader.h"
#include "Utilities/Logger.h"
#include <stdexcept>

ComputeShader::ComputeShader(const std::filesystem::path& shaderPath, const std::filesystem::path& binaryPath)
    : BaseShader(binaryPath), m_ShaderPath(shaderPath) {
    LoadShader();
}

void ComputeShader::ReloadShader() {
    if (*m_RendererIDPtr != 0) {
        glDeleteProgram(*m_RendererIDPtr);
        *m_RendererIDPtr = 0;
    }
    m_UniformLocationCache.clear();
    LoadShader(true);
}

void ComputeShader::LoadShader(bool reload) {
    if (!reload && !m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded compute shader binary from '{}'.", m_BinaryPath.string());
        return;
    }

    std::string source = ReadFile(m_ShaderPath);
    std::unordered_set<std::string> includedFiles;
    source = ResolveIncludes(source, m_ShaderPath.parent_path(), includedFiles);

    GLuint shader = CompileShader(GL_COMPUTE_SHADER, source);

    *m_RendererIDPtr = LinkProgram({ shader });

    if (!m_BinaryPath.empty()) {
        SaveBinary();
    }
}

void ComputeShader::Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
    Bind();
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    Unbind();
}