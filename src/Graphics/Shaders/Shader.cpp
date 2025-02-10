#include "Shader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

namespace Graphics {

    Shader::Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
        const std::filesystem::path& binaryPath)
        : BaseShader(binaryPath),
        m_ShaderStages(shaderStages)
    {
        LoadShaders(false);
    }

    void Shader::ReloadShader() {
        if (m_RendererID != 0) {
            glDeleteProgram(m_RendererID);
            m_RendererID = 0;
        }
        m_UniformLocationCache.clear();
        LoadShaders(true);
    }

    void Shader::LoadShaders(bool reload) {
        if (!reload && !m_BinaryPath.empty() && LoadBinary()) {
            Logger::GetLogger()->info("Loaded shader program from binary '{}'.", m_BinaryPath.string());
            return;
        }
        std::vector<GLuint> CompiledShaders;
        CompiledShaders.reserve(m_ShaderStages.size());
        try {
            for (const auto& [Type, Path] : m_ShaderStages) {
                std::string Src = ReadFile(Path);
                std::unordered_set<std::string> Included;
                Src = ResolveIncludes(Src, Path.parent_path(), Included);
                CompiledShaders.push_back(CompileShader(Type, Src));
            }
            m_RendererID = LinkProgram(CompiledShaders);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Error loading shaders: {}", e.what());
            throw;
        }
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to link shader program from sources.");
            return;
        }
        if (!m_BinaryPath.empty()) {
            try {
                SaveBinary();
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("Failed to save shader binary: {}", e.what());
            }
        }
        Logger::GetLogger()->info("Shader program created successfully, ID={}.", m_RendererID);
    }

} // namespace Graphics