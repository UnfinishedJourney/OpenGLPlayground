#include "ComputeShader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

namespace graphics {

    ComputeShader::ComputeShader(const std::filesystem::path& shaderPath,
        const std::filesystem::path& binaryPath)
        : BaseShader(binaryPath),
        m_ShaderPath(shaderPath)
    {
        LoadShader(false);
    }

    void ComputeShader::ReloadShader() {
        if (m_RendererID != 0) {
            glDeleteProgram(m_RendererID);
            m_RendererID = 0;
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
        GLuint shaderHandle = CompileShader(GL_COMPUTE_SHADER, source);
        if (!shaderHandle) {
            Logger::GetLogger()->error("Failed to compile compute shader '{}'.", m_ShaderPath.string());
            return;
        }
        m_RendererID = LinkProgram({ shaderHandle });
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to link compute shader program '{}'.", m_ShaderPath.string());
            return;
        }
        if (!m_BinaryPath.empty())
            SaveBinary();
        Logger::GetLogger()->info("Compute shader '{}' compiled and linked successfully.", m_ShaderPath.string());
    }

    void ComputeShader::Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
        Bind();
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
        Unbind();
    }

} // namespace graphics