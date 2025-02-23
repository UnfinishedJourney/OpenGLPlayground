#include "Shader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

namespace graphics {

    Shader::Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
        const std::filesystem::path& binaryPath)
        : BaseShader(binaryPath),
        m_ShaderStages_(shaderStages)
    {
        LoadShaders(false);
    }

    void Shader::ReloadShader() {
        if (m_RendererId_ != 0) {
            glDeleteProgram(m_RendererId_);
            m_RendererId_ = 0;
        }
        m_UniformLocationCache_.clear();
        LoadShaders(true);
    }

    void Shader::LoadShaders(bool reload) {
        if (!reload && !m_BinaryPath_.empty() && LoadBinary()) {
            Logger::GetLogger()->info("Loaded shader program from binary '{}'.", m_BinaryPath_.string());
            return;
        }
        std::vector<GLuint> compiledShaders;
        compiledShaders.reserve(m_ShaderStages_.size());
        try {
            for (const auto& [type, path] : m_ShaderStages_) {
                std::string src = ReadFile(path);
                std::unordered_set<std::string> included;
                src = ResolveIncludes(src, path.parent_path(), included);
                compiledShaders.push_back(CompileShader(type, src));
            }
            m_RendererId_ = LinkProgram(compiledShaders);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Error loading shaders: {}", e.what());
            throw;
        }
        if (m_RendererId_ == 0) {
            Logger::GetLogger()->error("Failed to link shader program from sources.");
            return;
        }
        if (!m_BinaryPath_.empty()) {
            try {
                SaveBinary();
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("Failed to save shader binary: {}", e.what());
            }
        }
        Logger::GetLogger()->info("Shader program created successfully, ID={}.", m_RendererId_);
    }

} // namespace graphics
