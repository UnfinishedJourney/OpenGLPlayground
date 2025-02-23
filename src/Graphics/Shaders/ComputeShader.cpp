#include "ComputeShader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

namespace graphics {

    ComputeShader::ComputeShader(const std::filesystem::path& shaderPath, const std::filesystem::path& binaryPath)
        : BaseShader(binaryPath), shaderPath_(shaderPath)
    {
        LoadShader(false);
    }

    void ComputeShader::ReloadShader() {
        if (rendererId_ != 0) {
            glDeleteProgram(rendererId_);
            rendererId_ = 0;
        }
        uniformLocationCache_.clear();
        LoadShader(true);
    }

    void ComputeShader::LoadShader(bool reload) {
        if (!reload && !binaryPath_.empty() && LoadBinary()) {
            Logger::GetLogger()->info("Loaded compute shader binary from '{}'.", binaryPath_.string());
            return;
        }
        std::string source = ReadFile(shaderPath_);
        std::unordered_set<std::string> includedFiles;
        source = ResolveIncludes(source, shaderPath_.parent_path(), includedFiles);
        GLuint shaderHandle = CompileShader(GL_COMPUTE_SHADER, source);
        if (!shaderHandle) {
            Logger::GetLogger()->error("Failed to compile compute shader '{}'.", shaderPath_.string());
            return;
        }
        rendererId_ = LinkProgram({ shaderHandle });
        if (rendererId_ == 0) {
            Logger::GetLogger()->error("Failed to link compute shader program '{}'.", shaderPath_.string());
            return;
        }
        if (!binaryPath_.empty())
            SaveBinary();
        Logger::GetLogger()->info("Compute shader '{}' compiled and linked successfully.", shaderPath_.string());
    }

    void ComputeShader::Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
        Bind();
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
        Unbind();
    }

} // namespace graphics